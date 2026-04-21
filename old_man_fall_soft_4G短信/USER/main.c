#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "mysoftiic.h"
#include "oled.h"
#include "gui.h"
#include "app.h"
#include "stmflash.h"
#include "menu.h"
#include "mainhmi.h"
#include "max30102.h"
#include "Mpu_6050.h"
#include "gps.h"
#include "sms.h"
#include "timer.h"
#include "string.h"
#include "ds18b20.h"
#include "gizwits_product.h" 
#include "sms4g.h"

#define KEY_SCAN_MS	20	// 20ms
#define MENU_SCAN_MS	500	// 50ms
#define MPU6050_SCAN_MS	500	// 500ms
#define BEEP_ALARM_MS	500	// 500ms
#define AUDIO_ALARM_MS	3000	// 500ms
#define GPS_SCAN_MS	100	// 50ms
#define SMS_SCAN_MS	100	// 50ms
#define DS18B20_SCAN_MS	2000	// 50ms
#define CLOUDPLATFORM_SCAN_MS	100	// 50ms

uint8_t gps_szbuf[MAX_CMD_LEN];
uint16_t gps_len;
uint8_t sms_szbuf[MAX_CMD_LEN];
uint16_t sms_len;


char sms_alarm_buf[256];
float mpu6050buf[3];


static MainMenuCfg_t tMainMenu;
static uint32_t key_scan_tick = 0;			// 按键计数
static uint32_t menu_scan_tick = 0;			// 
static uint32_t beep_alarm_tick = 0;
static uint32_t sms_alarm_tick = 0;
static uint32_t max30102_tick = 0;
static uint32_t max30102_ms = 1000;
static uint32_t mpu6050_tick = 0;
static uint32_t gps_data_tick = 0;
static uint32_t sms_data_tick = 0;
// static uint32_t sr04_data_tick = 0;
static uint32_t ds18b20_data_tick = 0;
static uint32_t adc_data_tick = 0;
static uint32_t cloudPlatform_tick = 0;
static uint32_t sr04_data_tick = 0;
static uint32_t audio_tick = 0;

const double EPS = 0.0000001;

uint8_t alarmFlag = 0;


//初始化独立看门狗
//prer:分频数:0~7(只有低 3 位有效!)
//分频因子=4*2^prer.但最大值只能是 256!
//rlr:重装载寄存器值:低 11 位有效.
//时间计算(大概):Tout=((4*2^prer)*rlr)/40 (ms).
void IWDG_Init(u8 prer,u16 rlr)
{
	// 1、取消寄存器写保护 写0x5555
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	// 2、设置独立看门狗预分频系数
	IWDG_SetPrescaler(prer);
	// 3、设置独立看门狗重装载值
	IWDG_SetReload(rlr);;
	// 4、重载计数值喂狗 写0xAAAA
	IWDG_ReloadCounter();
	// 5、启动看门狗     写0xCCCC
	IWDG_Enable();
}

// 清除看门狗
void IWDG_Feed(void)
{
	IWDG_ReloadCounter();
}


int main(void)
{
	static uint8_t fall_times = 0;

	delay_init();            // 延时函数初始化
	
	my_uart_recv_buf_init();		// 串口缓冲队列初始化
	
	
	// 串口初始化
	uart_init(38400);
	uart2_init(9600);
	uart3_init();
	
	cfg_init();
	
	gizwitsInit();

	// IIC初始化
	HAL_SOFT_I2C_Init(IIC_SOFT_NUM1, (uint32_t)GPIOB, GPIO_Pin_7, (uint32_t)GPIOB, GPIO_Pin_6);
	HAL_SOFT_I2C_Init(IIC_SOFT_NUM0, (uint32_t)GPIOB, GPIO_Pin_9, (uint32_t)GPIOB, GPIO_Pin_8);

	// LED管脚初始化
	gled.led_init();
	// 按键初始化
	gkey.key_init();

	// 心率血氧传感器初始化
	max30102_init();

	// 摔倒传感器初始化
	InitMPU6050();

	// 短信初始化
	// sms_control_init(&g_smsControl);


	// 屏幕初始化
	OLED_Init();
	OLED_Clear();
	GUI_ClearMap(0x00);
	


	// 菜单初始化
	tMainMenu.pszDesc = "主菜单";
	tMainMenu.pszEnDesc = "Main Menu";
	tMainMenu.pfnLoadCallFun = Hmi_LoadMainHmi;
	tMainMenu.pfnRunCallFun = Hmi_MainTask;
	Menu_Init(&tMainMenu);



	
	gled.led_init();
	gkey.key_init();
	
	

	// 看门狗初始化
	// IWDG_Init(4, 625);
	
	
	DS18B20_Init();

	while (1)
	{
		if (millis_elapsed(key_scan_tick) >= KEY_SCAN_MS)
		{
			key_scan_tick = systicks_get();

			gkey.key_scan();
		}

		if (millis_elapsed(menu_scan_tick) >= MENU_SCAN_MS)
		{
			menu_scan_tick = systicks_get();

			Menu_Task();
		}

		if (millis_elapsed(sms_alarm_tick) >= (10 * 1000))
		{
			sms_alarm_tick = systicks_get();
			
			alarmFlag = 0;

			printf("短信报警判断\r\n");

			if (g_appdata.fall == 0)
			{
				alarmFlag = 1;
			}
			
			if (((g_appdata.HR < cfg_data.hr_min) || (g_appdata.HR > cfg_data.hr_max)) && (g_appdata.HR != 0))
			{
				alarmFlag = 1;
				g_appdata.hrSta = 1;
			}
			else
			{
				g_appdata.hrSta = 0;
			}
			
			if (((g_appdata.SpO2 < cfg_data.sqo2_min)) && ((g_appdata.SpO2 - 0.0) > EPS))
			{
				alarmFlag = 1;
				g_appdata.xueyangSta = 1;
			}
			else
			{
				g_appdata.xueyangSta = 0;
			}	
			
			if (g_appdata.temp > cfg_data.wendu_max)
			{
				alarmFlag = 1;
				g_appdata.tempSta = 1;
			}
			else
			{
				g_appdata.tempSta = 0;
			}

			if (alarmFlag == 1)
			{
				if (g_appdata.mistakeTouchSta == 0)
				{
					send_sms_alarm();
					g_appdata.alarm = 1;			// 表示报警状态
				}
				else		// 正常
				{
					g_appdata.mistakeTouchSta = 0;
				}
			}
		
		}

		if (millis_elapsed(beep_alarm_tick) >= BEEP_ALARM_MS)
		{
			beep_alarm_tick = systicks_get();


			alarmFlag = 0;

			if (g_appdata.fall == 0)
			{
				alarmFlag = 1;
			}
			else if (((g_appdata.HR < cfg_data.hr_min) || (g_appdata.HR > cfg_data.hr_max)) && (g_appdata.HR != 0))
			{
				alarmFlag = 1;
			}
			else if (((g_appdata.SpO2 < cfg_data.sqo2_min)) && ((g_appdata.SpO2 - 0.0) > EPS))
			{
				alarmFlag = 1;
			}
			else if (g_appdata.temp > cfg_data.wendu_max)
			{
				alarmFlag = 1;
			}
			// else if (g_appdata.hc_sr04_distance < 5.0)
			// {
			// 	alarmFlag = 1;
			// }

			if ((alarmFlag == 1) && (g_appdata.mistakeTouchSta == 0))
			{
				g_appdata.alarm = 1;			// 表示报警状态
				gled.led_open(LED1);
				gled.led_close(LED2);
				gled.led_open(LED3);
			}
			else
			{
				gled.led_close(LED1);
				gled.led_open(LED2);
				gled.led_close(LED3);
			}

			sms_fsm_handler();
		}


		// 如果没有手指没有放上那就1秒一次
		if (millis_elapsed(max30102_tick) >= max30102_ms)
		{
			max30102_tick = systicks_get();
			heart_data_get();
			
			// 判断手指是否在MAX30102上，如果不在，那就1S检测一次，如果在，那就一直采集。
			if (get_sw_status() == 1)
			{
				max30102_ms = 10;
			}
			else
			{
				max30102_ms = 1000;
			}
		}

		if (millis_elapsed(mpu6050_tick) >= MPU6050_SCAN_MS)
		{
			mpu6050_tick = systicks_get();

			mpu6050_get_accelangle(mpu6050buf);

			g_appdata.a = mpu6050buf[0];
			g_appdata.b = mpu6050buf[1];
			g_appdata.c = mpu6050buf[2];

			// if (((g_appdata.a > 40.0) && (g_appdata.a < 59.0)) || ((g_appdata.a > 200.0) && (g_appdata.a < 220.0)) )
			if (((g_appdata.a > 40.0) && (g_appdata.a < 59.0)))
			{
				g_appdata.stepnum++;
			}

			if ((g_appdata.a > 60.0) && (g_appdata.a < 220.0))
			{
				fall_times++;

				if (fall_times > 10)
				{
					g_appdata.fall = 0;
				}
			}
			else
			{
				fall_times  = 0;
				g_appdata.fall = 1;
			}
		}


		if (millis_elapsed(ds18b20_data_tick) >= DS18B20_SCAN_MS)
		{
			ds18b20_data_tick = systicks_get();
			
			g_appdata.temp = DS18B20_Get_Temp() / 10;

		}

		if (millis_elapsed(gps_data_tick) >= GPS_SCAN_MS)
		{
			gps_data_tick = systicks_get();

			gps_len = queue_read(&my_uart2_rx_Q, gps_szbuf, sizeof(gps_szbuf));

			// printf("%s", gps_szbuf);
			my_gps_handle_rx((uint8_t *)gps_szbuf, gps_len);
			
			gps_to_gcj(gpsdata.latitude, gpsdata.longitude, &g_appdata.gpsdata.gcj_lat, &g_appdata.gpsdata.gcj_lng);
		}
		
		if (millis_elapsed(cloudPlatform_tick) >= CLOUDPLATFORM_SCAN_MS)
		{
			cloudPlatform_tick = systicks_get();
			if (g_appdata.wifiSta == 1)
				userHandle();
			gizwitsHandle((dataPoint_t *)&currentDataPoint);
		}

		// IWDG_Feed();
	}
}


