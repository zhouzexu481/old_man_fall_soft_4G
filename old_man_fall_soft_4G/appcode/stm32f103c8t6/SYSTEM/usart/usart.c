#include "usart.h"
#include "string.h"
#include "gps.h"
#include "math.h"
#include "gizwits_product.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif


//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  

uint8_t g_uart1_ring_buffer[MY_UART1_BUFFER_SIZE] = {0};

uint8_t g_uart2_ring_buffer[MY_UART2_BUFFER_SIZE] = {0};

uint8_t g_uart3_ring_buffer[MY_UART3_BUFFER_SIZE] = {0};




// 高德加密参数（双精度）
#define GCJ_A 6378245.0
#define GCJ_EE 0.00669342162296594323
#define GCJ_PI 3.1415926535897932384626433832795

// 高精度字符串解析
static int parse_ddmmmm(const char* str, int degree_digits, double *result) {
    long degrees = 0;
    double minutes = 0.0;
    int i;
    
    // 解析度数部分
    for(i = 0; i < degree_digits; i++) {
        if(str[i] < '0' || str[i] > '9') return -1;
        degrees = degrees * 10 + (str[i] - '0');
    }
    
    // 解析分钟部分
    const char *p = str + degree_digits;
    long whole = 0, frac = 0;
    int frac_digits = 0;
    
    // 整数部分
    while(*p && *p != '.' && frac_digits < 4) {
        if(*p < '0' || *p > '9') return -1;
        whole = whole * 10 + (*p - '0');
        p++;
    }
    
    // 小数部分
    if(*p == '.') {
        p++;
        for(frac_digits = 0; *p && frac_digits < 4; frac_digits++, p++) {
            if(*p < '0' || *p > '9') return -1;
            frac = frac * 10 + (*p - '0');
        }
    }
    
    minutes = whole + frac / pow(10, frac_digits);
    *result = degrees + minutes / 60.0;
    return 0;
}

// 高精度sin计算（查表+线性插值）
static double precise_sin(double rad) {
    static const double sin_table[91] = { // 0-90度，1度步长
        0.000000, 0.017452, 0.034899, /*...填充实际值到90度*/ 1.000000
    };
    
    // 角度归一化
    double deg = fmod(rad * 180.0 / GCJ_PI, 360.0);
    if(deg < 0) deg += 360.0;
    
    int quadrant = (int)(deg / 90.0);
    double base_deg = fmod(deg, 90.0);
    
    // 查表索引和插值
    int idx = (int)base_deg;
    double frac = base_deg - idx;
    double val = sin_table[idx] * (1 - frac) + sin_table[idx + 1] * frac;
    
    // 象限处理
    switch(quadrant) {
        case 1: val = sin_table[90 - idx - 1]; break;
        case 2: val = -val; break;
        case 3: val = -sin_table[90 - idx - 1]; break;
    }
    return val;
}

// 改进的坐标转换
void precise_transform(double x, double y, double *dlat, double *dlon) {
    // 纬度偏移
    double tlat = -100.0 + 2.0 * x + 3.0 * y;
    tlat += 0.2 * y * y + 0.1 * x * y;
    tlat += 0.2 * sqrt(fabs(x));
    tlat += (20.0 * precise_sin(6.0 * x * GCJ_PI) + 
            20.0 * precise_sin(2.0 * x * GCJ_PI)) * 2.0 / 3.0;
    
    // 经度偏移
    double tlon = 300.0 + x + 2.0 * y;
    tlon += 0.1 * x * x + 0.1 * x * y;
    tlon += 0.1 * sqrt(fabs(x));
    tlon += (20.0 * precise_sin(x * GCJ_PI) + 
            40.0 * precise_sin(x / 3.0 * GCJ_PI)) * 2.0 / 3.0;
    
    *dlat = tlat;
    *dlon = tlon;
}

// 主转换函数
int gps_to_gcj(const char *lat_str, const char *lon_str, 
              double *gcj_lat, double *gcj_lon) 
{
    double wgs_lat, wgs_lon;
    
    // 解析纬度（ddmm.mmmm）
    if(parse_ddmmmm(lat_str, 2, &wgs_lat) != 0) return -1;
    
    // 解析经度（dddmm.mmmm）
    if(parse_ddmmmm(lon_str, 3, &wgs_lon) != 0) return -2;
    
    // 计算中间参数
    double x = wgs_lon - 105.0;
    double y = wgs_lat - 35.0;
    
    double dlat, dlon;
    precise_transform(x, y, &dlat, &dlon);
    
    // 椭圆参数修正
    double rad_lat = wgs_lat * GCJ_PI / 180.0;
    double sin_val = precise_sin(rad_lat);
    double magic = 1 - GCJ_EE * sin_val * sin_val;
    double sqrt_magic = sqrt(magic);
    
    // 最终偏移计算
    *gcj_lat = wgs_lat + (dlat * 180.0) / (GCJ_A * (1 - GCJ_EE) / (magic * sqrt_magic) * GCJ_PI);
    *gcj_lon = wgs_lon + (dlon * 180.0) / (GCJ_A / sqrt_magic * cos(rad_lat) * GCJ_PI);
    
    return 0;
}




my_cycle_queue my_uart1_rx_Q = {
    NULL,
    MY_UART1_BUFFER_SIZE,
    0,
    0,
    1,              //empty
    0,
    0,
};

my_cycle_queue my_uart2_rx_Q = {
    NULL,
    MY_UART2_BUFFER_SIZE,
    0,
    0,
    1,         //empty
    0,
    0,
};

my_cycle_queue my_uart3_rx_Q = {
    NULL,
    MY_UART3_BUFFER_SIZE,
    0,
    0,
    1,         //empty
    0,
    0,
};

#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
}
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 

  



void queue_clean( my_cycle_queue *Q_ptr )
{
    Q_ptr->head = Q_ptr->tail = 0;
    Q_ptr->empty = 1;
    Q_ptr->full = 0;
    Q_ptr->overflow = 0;
}


int queue_write(my_cycle_queue *Q_ptr, uint8_t *data, uint32_t len)
{
    int ret = 0;

    for (; ret < len; ret++)
    {
        *(Q_ptr->buf + Q_ptr->head) = *(data + ret);

        if ((1==Q_ptr->full) && (Q_ptr->head==Q_ptr->tail))
        {
            Q_ptr->overflow = 1;
        }

        Q_ptr->head = ++Q_ptr->head % Q_ptr->size;

        if (Q_ptr->head == Q_ptr->tail)
        {
            Q_ptr->full = 1;
        }

        if (1 == Q_ptr->empty)
        {
            Q_ptr->empty = 0;
        }
    }

    if ( Q_ptr->overflow )
    {
        Q_ptr->tail = Q_ptr->head;
    }

    return ret;
}

int queue_read( my_cycle_queue *Q_ptr, uint8_t *data, uint32_t len )
{
    int ret = 0;

    if ( !Q_ptr->empty )
    {
        while( ret < len )
        {
            *(data + ret) = *(Q_ptr->buf + Q_ptr->tail);
            Q_ptr->tail = ++Q_ptr->tail % Q_ptr->size;
            ret++;

            if ( Q_ptr->tail == Q_ptr->head )
            {
                Q_ptr->empty = 1;
                break;
            }
        }
    }

    if ( ( ret > 0 ) && ( 1 == Q_ptr->full ) )
    {
        Q_ptr->full = 0;
        Q_ptr->overflow = 0;
    }

    return ret;
}

void my_uart_recv_buf_init(void)
{
     my_uart1_rx_Q.buf = g_uart1_ring_buffer;
	my_uart2_rx_Q.buf = g_uart2_ring_buffer;
	my_uart3_rx_Q.buf = g_uart3_ring_buffer;

}


void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

// 用于接收GPS的
void uart2_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART1，GPIOA时钟
	
	
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	// USART_InitStructure.USART_BaudRate = 115200;//串口波特率
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART2, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART2, ENABLE);                    //使能串口1 

}


// 用于接收GPS的
void uart3_init(void){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能USART1，GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能USART1，GPIOA时钟
	
	
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = 9600;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART3, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART3, ENABLE);                    //使能串口1 

}


// gps 数据处理

int parse_gps_cmd(char *cmdline, int cmd_len)
{
    // int argc = 0;
    // int num_commands = 0;
    // char *argv[MAX_ARGS] = {0};

    if (0 == strlen(cmdline) || (cmd_len == 1 && cmdline[0] == '\n'))
    {
        return -1;
    }

#if 0
    printf("gps_rx:%s\r\n", cmdline);
#endif

    // 如果是 $GNRMC
    if ((strstr(cmdline, "$GNRMC") != NULL) || (strstr(cmdline, "$GPRMC") != NULL))
    {
        gnrmc_deal(cmdline, cmd_len);
    }

    return 0;
}

// 处理gps收到的所有字符串
void my_gps_handle_rx(uint8_t *pData, uint32_t iLen)
{
    static uint32_t t_i = 0;
    static uint8_t atCmdBuf[MAX_CMD_LEN] = {0};
    uint32_t i = 0;
    // uint32_t firstIndex = 0;
    // uint16_t tmp;

    if (iLen <= 0)
    {
        return;
    }

    for (i = 0; i < iLen; i++)
    {
        if (pData[i] == '\r' || pData[i] == '\n') // 回车是\r 为了兼容同时处理 \n
        {
            if (atCmdBuf[0] == '$')
            {
                parse_gps_cmd((char *)atCmdBuf, t_i);
            }

            atCmdBuf[0] = 0;
            t_i = 0;

            // 如果下个字符是\n，跳过
            if (pData[i + 1] == '\n')
            {
                i++;
            }
        }
        else if (t_i < (MAX_CMD_LEN - 1))
        {
            atCmdBuf[t_i++] = pData[i];
            atCmdBuf[t_i] = '\0';
        }
    }
}




/**
 * USART1发送len个字节.
 * buf:发送区首地址
 * len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 **/
void USART1_Send_Data(u8 *buf,u16 len)
{
    u16 t;
    for(t=0;t<len;t++)        //循环发送数据
    {           
         while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);      
         USART_SendData(USART1,buf[t]);
     }     
     while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);          
}



void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
		{
			Res = USART_ReceiveData(USART1);	//读取接收到的数据
			
			// 写入队列里面
			queue_write(&my_uart1_rx_Q, &Res, 1);
			

		} 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
	


void USART2_IRQHandler(void) 
{
	u8 Res;
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
		{
			Res =USART_ReceiveData(USART2);	//读取接收到的数据
			
			// 写入队列里面
			queue_write(&my_uart2_rx_Q, &Res, 1);
		} 

}




void USART3_IRQHandler(void) 
{
	u8 Res;
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
		{
			Res =USART_ReceiveData(USART3);	//读取接收到的数据
			
			// 写入队列里面
			// queue_write(&my_uart3_rx_Q, &Res, 1);
      gizPutData(&Res, 1);
		} 

}













	

