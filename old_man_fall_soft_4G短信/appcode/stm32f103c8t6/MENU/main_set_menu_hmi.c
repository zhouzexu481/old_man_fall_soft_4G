#include "hmi_common.h"
#include "menu.h"
#include "stmflash.h"
#include "gui.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"






MenuList_t sg_set_menuTable[] = 
{
    {"设置项1", "settable1",  NULL, NULL, NULL, "settable1"},
    {"设置项2", "settable2",  NULL, NULL, NULL, "settable2"},
    {"设置项3", "settable3",  NULL, NULL, NULL, "settable3"},
    {"设置项4", "settable4",  NULL, NULL, NULL, "settable4"},
    {"设置项5", "settable5",  NULL, NULL, NULL, "settable5"},
    {"设置项6", "settable6",  NULL, NULL, NULL, "settable6"},
		{"设置项7", "settable7",  NULL, NULL, NULL, "settable7"},

};



/* 主菜单显示效果 */
static void showmain_status_menu(MenuShow_t *ptShowInfo)
{
#define SET_MENU_HR_MAX "心率上限:%d次"
#define SET_MENU_HR_MIN "心率下限:%d次"
#define SET_MENU_SQO2_MIN "血氧下限:%d%%"
#define SET_MENU_WENDU_MAX "温度上限:%d℃"
#define SET_MENU_WIFI "WIFI配置"
#define SET_MENU_SMS_PHONE "电话:%s"
#define SET_MENU_EXIT "退出"


    // char * p[7] = {0};
    char oledShowBuf[128] = {0};
    u8 mode1 = Normal;
    u8 mode2 = Normal;
    u8 mode3 = Normal;
    u8 mode4 = Normal;
    u8 mode5 = Normal;
    u8 mode6 = Normal;
		u8 mode7 = Normal;

    // u8 mode8 = Normal;
    // uint8_t i = 0;


    // p[0] = SET_MENU_HR_MAX;
    // p[1] = SET_MENU_HR_MIN;
    // p[2] = SET_MENU_SQO2_MAX;
    // p[3] = SET_MENU_SQO2_MIN;
    // p[4] = SET_MENU_SMS_ALARM_S;
    // p[5] = SET_MENU_SMS_PHONE;
    // p[6] = SET_MENU_EXIT;

    if (ptShowInfo->selectItem == 0)	// 设置项1
    {
        mode1 = Reverse;
    }
    else if (ptShowInfo->selectItem == 1)   // 设置项2
    {
        mode2 = Reverse;
    }
    else if (ptShowInfo->selectItem == 2)   // 设置项3
    {
        mode3 = Reverse;
    }
    else if (ptShowInfo->selectItem == 3)   // 设置项4
    {
        mode4 = Reverse;
    }
    else if (ptShowInfo->selectItem == 4)   // 设置项5
    {
        mode5 = Reverse;
    }
    else if (ptShowInfo->selectItem == 5)   // 设置项6
    {
        mode6 = Reverse;
    }
		else if (ptShowInfo->selectItem == 6)   // 设置项6
    {
        mode7 = Reverse;
    }

    // else if (ptShowInfo->selectItem == 7)   // 设置项8
    // {
    //     mode8 = Reverse;
    // }

    GUI_ClearMap(0x00);

    if ((ptShowInfo->selectItem == 0) || (ptShowInfo->selectItem == 1) || 
        (ptShowInfo->selectItem == 2) || (ptShowInfo->selectItem == 3))
    {
        memset(oledShowBuf, 0, sizeof(oledShowBuf));
        sprintf(oledShowBuf, SET_MENU_HR_MAX, cfg_data.hr_max);
        GUI_printf(0, 0, oledShowBuf, mode1);

        memset(oledShowBuf, 0, sizeof(oledShowBuf));
        sprintf(oledShowBuf, SET_MENU_HR_MIN, cfg_data.hr_min);
        GUI_printf(0, 16, oledShowBuf, mode2);

        memset(oledShowBuf, 0, sizeof(oledShowBuf));
        sprintf(oledShowBuf, SET_MENU_SQO2_MIN, cfg_data.sqo2_min);
        GUI_printf(0, 32, oledShowBuf, mode3);

        memset(oledShowBuf, 0, sizeof(oledShowBuf));
        sprintf(oledShowBuf, SET_MENU_WENDU_MAX, cfg_data.wendu_max);
        GUI_printf(0, 48, oledShowBuf, mode4);
    }
    else if ((ptShowInfo->selectItem == 4) || (ptShowInfo->selectItem == 5) || (ptShowInfo->selectItem == 6))
    {
        memset(oledShowBuf, 0, sizeof(oledShowBuf));
        sprintf(oledShowBuf, SET_MENU_WIFI);
        GUI_printf(0, 0, oledShowBuf, mode5);

        memset(oledShowBuf, 0, sizeof(oledShowBuf));
        sprintf(oledShowBuf, SET_MENU_SMS_PHONE, cfg_data.phone);
        GUI_printf(0, 16, oledShowBuf, mode6);

        memset(oledShowBuf, 0, sizeof(oledShowBuf));
        sprintf(oledShowBuf, SET_MENU_EXIT);
        GUI_printf(0, 32, oledShowBuf, mode7);
    }


    GUI_ShowMap();
}


void hmi_main_set_menu_load(void)
{
    Menu_Bind(sg_set_menuTable, GET_MENU_NUM(sg_set_menuTable), showmain_status_menu);
}







