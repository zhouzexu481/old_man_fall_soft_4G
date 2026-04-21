#ifndef __OLED_H
#define __OLED_H
#include "sys.h"
#include "mysoftiic.h"

#define Width		128
#define Height	64
#define Pages		8


//OLED指令
#define OLED_CMD  0		//写命令0
#define OLED_DATA 1	//写数据1


void OLED_Write_IIC_Command(u8 IIC_Command);
void OLED_Write_IIC_Data(u8 IIC_Data);
void OLED_WR_Byte(u8 dat,u8 cmd);           //写入数据

void OLED_ShowPic(u8* pic);															//显示Pic

void OLED_Set_Pos(u8 x, u8 y);					//坐标设置
void OLED_Display_On(void);             //开启OLED显示
void OLED_Display_Off(void);	   				//关闭OLED显示
void OLED_Init(void);                   //初始化SSD1306
void OLED_Clear(void);                  //清屏函数
void OLED_Clear_page(u8 i);             //清页函数,清除第i(1<=i<=8)页
void OLED_SetBrightness(u8 brightness);	//设置屏幕亮度


#endif
