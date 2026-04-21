#include "menu.h"
#include "hmi_common.h"
#include "key.h"
#include "stdio.h"
#include "string.h"
#include "stmflash.h"
#include "gizwits_product.h"
#include "app.h"
#include "sms4g.h"


// 主菜单按键回调
static void main_menu_KeyFunCB(uint8_t key)
{
    // 状态界面
    if ((MenuManage_get()->pMenuCtrl->selectItem == 0) ||
        (MenuManage_get()->pMenuCtrl->selectItem == 1))
    {
        if (key == KEY3)
        {
			Menu_SelectNext(False);
            
        }
        else if (key == KEY1)
        {
            Menu_SelectPrevious(False);
        }
		else if (key == KEY5)
		{
			send_sms_alarm();
		}
    }
    else if (MenuManage_get()->pMenuCtrl->selectItem == 2)
    {
        if (key == KEY5)
        {
            Menu_Enter();
        }
        else if (key == KEY1)
        {
			Menu_SelectPrevious(False);
        }
    }

    // printf("itemsNum:%d\r\n", MenuManage_get()->pMenuCtrl->itemsNum);
    // printf("selectItem:%d\r\n", MenuManage_get()->pMenuCtrl->selectItem);
}


// 二级菜单set回调
static void main_set_menu_KeyFunCB(uint8_t key)
{
    // 状态界面

    if (key == KEY4)
    {
        Menu_SelectNext(False);
        
    }
    else if (key == KEY2)
    {
        Menu_SelectPrevious(False);
    }
    

    if (MenuManage_get()->pMenuCtrl->selectItem == 0)
    {
        if (key == KEY3)
        {
            if (cfg_data.hr_max >= HR_MAX)
            {
                cfg_data.hr_max = HR_MAX;
            }
            else
            {
                cfg_data.hr_max = cfg_data.hr_max + 1;
                cfg_restore();
            }
        }
        else if (key == KEY1)
        {
            if (cfg_data.hr_max <= 120)  // 心跳上线报警不要低于120
            {
                cfg_data.hr_max = 120;
            }
            else
            {
                cfg_data.hr_max = cfg_data.hr_max - 1;
                cfg_restore();
            }
            
        }
    }
    else if (MenuManage_get()->pMenuCtrl->selectItem == 1)
    {
        if (key == KEY3)
        {
            if (cfg_data.hr_min >= HR_MIN)
            {
                cfg_data.hr_min = HR_MIN;
            }
            else
            {
                cfg_data.hr_min = cfg_data.hr_min + 1;
                cfg_restore();
            }
        }
        else if (key == KEY1)
        {
            if (cfg_data.hr_min <= 0)  // 心跳下限报警不要低于0
            {
                cfg_data.hr_min = 0;
            }
            else
            {
                cfg_data.hr_min = cfg_data.hr_min - 1;
                cfg_restore();
            }
        }
    }
    else if (MenuManage_get()->pMenuCtrl->selectItem == 2)
    {
        if (key == KEY3)
        {
            if (cfg_data.sqo2_min >= 100)
            {
                cfg_data.sqo2_min = 100;
            }
            else
            {
                cfg_data.sqo2_min = cfg_data.sqo2_min + 1;
                cfg_restore();
            }
        }
        else if (key == KEY1)
        {
            if (cfg_data.sqo2_min < 0)
            {
                cfg_data.sqo2_min = 0;
            }
            else
            {
                cfg_data.sqo2_min = cfg_data.sqo2_min - 1;
                cfg_restore();
            }
        }
    }
    else if (MenuManage_get()->pMenuCtrl->selectItem == 3)
    {
        if (key == KEY3)
        {
            if (cfg_data.wendu_max >= 43)
            {
                cfg_data.wendu_max = 43;
            }
            else
            {
                cfg_data.wendu_max = cfg_data.wendu_max + 1;
                cfg_restore();
            }
        }
        else if (key == KEY1)
        {
            if (cfg_data.wendu_max <= 33)  // 心跳下限报警不要低于90.0
            {
                cfg_data.wendu_max = 33;
            }
            else
            {
                cfg_data.wendu_max = cfg_data.wendu_max - 1;
                cfg_restore();
            }
        }
    }
		else if (MenuManage_get()->pMenuCtrl->selectItem == 4)
    {
        if (key == KEY5)
        {
            printf("WiFi重新配网\r\n");
						gizwitsSetMode(WIFI_AIRLINK_MODE);  // 配网
        }
    }
    else if (MenuManage_get()->pMenuCtrl->selectItem == 6)
    {
        if (key == KEY5)
        {
            Menu_Exit(True);
        }
    }

    // printf("itemsNum:%d\r\n", MenuManage_get()->pMenuCtrl->itemsNum);
    // printf("selectItem:%d\r\n", MenuManage_get()->pMenuCtrl->selectItem);
}

// 对外按键接口，将按键的编号和按键事件传进来
void menu_key_scan_cb(u8 keyid, u8 keyevent)
{
    // 按下按键
    if ((keyevent == FLEX_BTN_PRESS_DOWN))
    {
		// 如果为报警状态
        if (g_appdata.alarm == 1)
        {
            if (g_appdata.mistakeTouchSta == 0)
            {
                g_appdata.mistakeTouchSta = 1;
                g_appdata.alarm = 0;
            }
        }
		
		
        if (strcmp(MenuManage_get()->pMenuCtrl->pszEnDesc, "Main Menu") == 0)
        {
            main_menu_KeyFunCB(keyid);
        }
        else if (strcmp(MenuManage_get()->pMenuCtrl->pszEnDesc, "set_menu") == 0)
        {
            main_set_menu_KeyFunCB(keyid);
        }
    }
}




// 以下是提供给外面的api调用的程序，不同的软件，界面不同
void prompt__menu_set(void) // 心跳采集提示界面
{
    MenuManage_get()->pMenuCtrl->selectItem = 3;
    Menu_Task();
}

void status__menu_set(void) // 返回状态界面界面
{
    MenuManage_get()->pMenuCtrl->selectItem = 1;
}












