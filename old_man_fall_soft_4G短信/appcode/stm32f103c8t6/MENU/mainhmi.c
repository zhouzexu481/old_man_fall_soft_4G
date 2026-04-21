#include "menu.h"
#include "hmi_common.h"
#include "app.h"
#include "adc.h"
#include "stmflash.h"
#include "gui.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "gps.h"
#include "app.h"

char oledShowBuf[128] = {0};

static bool sg_isInit = false;

void hmi_status_menu_load(void);
void hmi_data_menu_load(void);
void hmi_set_menu_load(void);


/* 主菜单 */
MenuList_t sg_MainMenuTable[] = 
{
	{"状态界面", "status_menu",  hmi_status_menu_load, NULL, NULL, NULL},
	{"数据界面", "data_menu", 	NULL, NULL, NULL, NULL},
	{"设置界面", "set_menu",  hmi_main_set_menu_load, NULL, NULL, NULL},
	{"提示界面", "prompt_menu",  NULL, NULL, NULL, NULL},
};


void hmi_status_menu_load(void)
{
	GUI_ClearMap(0x00);

	// GUI_ShowBMP(48, 0, 80 - 1, 32 - 1 , car_off, Normal);

	memset(oledShowBuf, 0, sizeof(oledShowBuf));
	sprintf(oledShowBuf, "老人监护系统"); 
	GUI_Center_str(0, oledShowBuf, Normal);

//	if (g_appdata.fall == 1 || g_appdata.mistakeTouchSta)
//	{
//		GUI_Center_Bmp(16, 32, 32, oldmen, Normal);
//	}
//	else
//	{
//		GUI_Center_Bmp(16, 32, 32, oldmen_fall, Normal);
//	}
	
	if (g_appdata.fall == 1 || g_appdata.mistakeTouchSta)
	{
		
	}
	else
	{
		memset(oledShowBuf, 0, sizeof(oledShowBuf));
		sprintf(oledShowBuf, "摔倒!!!"); 
		GUI_Center_str(32, oledShowBuf, Normal);
	}

	memset(oledShowBuf, 0, sizeof(oledShowBuf));
	sprintf(oledShowBuf, "体温:%d 步数:%d", g_appdata.temp, g_appdata.stepnum);
	GUI_Center_str(48, oledShowBuf, Normal);


	GUI_ShowMap();
	
}

void hmi_set_menu_load(void)
{
	GUI_ClearMap(0x00);
	GUI_Center_Bmp(16, 64, 32, SETUP, Normal);
	GUI_Center_str(48, "设置", Normal);
	GUI_ShowMap();
}


void hmi_data_menu_load(void)
{
	GUI_ClearMap(0x00);

	memset(oledShowBuf, 0, sizeof(oledShowBuf));
	sprintf(oledShowBuf, "心跳:%d次/分", g_appdata.HR);
	GUI_printf(0, 0, oledShowBuf, Normal);

	memset(oledShowBuf, 0, sizeof(oledShowBuf));
	sprintf(oledShowBuf, "血氧:%0.1f%%", g_appdata.SpO2);
	GUI_printf(0, 16, oledShowBuf, Normal);

	memset(oledShowBuf, 0, sizeof(oledShowBuf));
	sprintf(oledShowBuf, "经度:%0.4f", g_appdata.gpsdata.gcj_lng);
	GUI_printf(0, 32, oledShowBuf, Normal);

	memset(oledShowBuf, 0, sizeof(oledShowBuf));
	sprintf(oledShowBuf, "纬度:%0.4f", g_appdata.gpsdata.gcj_lat);
	GUI_printf(0, 48, oledShowBuf, Normal);

	GUI_ShowMap();
}

void hmi_prompt_menu_load(void)
{
	GUI_ClearMap(0x00);
	GUI_Center_Bmp(16, 64, 32, HEARTBEAT, Normal);
	GUI_Center_str(48, "心跳血氧采集中", Normal);
	GUI_ShowMap();
}



/* 主菜单显示效果 */
static void ShowMainMenu(MenuShow_t *ptShowInfo)
{
	// float temp;
	// const double EPS = 0.0000001;
	// static uint16_t HR = 0;
	// static float SpO2 = 0.0;
//	u8 fall = 0;


	if (ptShowInfo->selectItem == 0)	// 状态界面
	{

		// if (g_appdata.a > 60.0 || g_appdata.b > 60.0 || g_appdata.c > 150.0)
		// if ((g_appdata.a > 60.0) && (g_appdata.a < 220.0))
		// {
		// 	g_appdata.fall = 0;
		// }
		// else
		// {
		// 	g_appdata.fall = 1;
		// }
		
		hmi_status_menu_load();

	}
	else if (ptShowInfo->selectItem == 1)	// 数据界面
	{
		hmi_data_menu_load();
	}
	else if (ptShowInfo->selectItem == 2)	// 设置界面
	{
		hmi_set_menu_load();
	}
	else if (ptShowInfo->selectItem == 3)
	{
		hmi_prompt_menu_load();
	}
	
	
}



void Hmi_LoadMainHmi(void)
{    
    Menu_Bind(sg_MainMenuTable, GET_MENU_NUM(sg_MainMenuTable), ShowMainMenu);
    sg_isInit = true;
}

void Hmi_MainTask(void)
{
    if (sg_isInit)
    {
        sg_isInit = false;
    }
    
}
