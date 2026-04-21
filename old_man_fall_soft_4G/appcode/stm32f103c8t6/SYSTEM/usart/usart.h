#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
#define  MY_UART1_BUFFER_SIZE    (1024)          // 串口1用于串口调试
#define  MY_UART2_BUFFER_SIZE    (1024)         // 串口2用于蓝牙AT指令接收
#define  MY_UART3_BUFFER_SIZE    (256)          // 串口3用于语音播放，暂时未使用串口接收


#define	MAX_CMD_LEN		256
#define	MAX_ARGS		10

typedef struct {
    uint8_t *buf;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
    unsigned empty: 1;
    unsigned full:  1;
    unsigned overflow: 1;  //queue input after full, overwrite occur
} my_cycle_queue;
	  	

extern my_cycle_queue my_uart1_rx_Q;
extern my_cycle_queue my_uart2_rx_Q;
extern my_cycle_queue my_uart3_rx_Q;

//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void uart2_init(u32 bound);
void uart3_init(void);

void queue_clean( my_cycle_queue *Q_ptr );
int queue_write(my_cycle_queue *Q_ptr, uint8_t *data, uint32_t len);
int queue_read( my_cycle_queue *Q_ptr, uint8_t *data, uint32_t len );
void my_uart_recv_buf_init(void);

void my_gps_handle_rx(uint8_t *pData, uint32_t iLen);

int gps_to_gcj(const char *lat_str, const char *lon_str, 
              double *gcj_lat, double *gcj_lon);
#endif


