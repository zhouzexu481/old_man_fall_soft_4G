#ifndef HMI_COMMON_H
#define HMI_COMMON_H

#include "sys.h"

// 按键回调
extern void menu_key_scan_cb(u8 keyid, u8 keyevent);


// 主菜单下的两个菜单页面
extern void hmi_main_set_menu_load(void);
extern void hmi_main_data_menu_load(void);


extern void prompt__menu_set(void);
extern void status__menu_set(void);

#endif
