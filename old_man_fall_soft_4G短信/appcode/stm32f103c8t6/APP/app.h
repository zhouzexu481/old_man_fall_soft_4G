#ifndef __APP_H_
#define __APP_H_

#include "sys.h"

// 坐标结构体
typedef struct {
    double lng; // 经度
    double lat; // 纬度
	
	double gcj_lng; // 经度
    double gcj_lat; // 纬度
} Coordinate;


typedef struct app
{
	u8 wifiSta;			// 1表示连接WIFI

	u8 mistakeTouchSta;     // 误触标志
    u8 alarm;           // 报警状态
	
	float a;
	float b;
	float c;

    // 心跳，血氧
    uint16_t HR;
	float SpO2;

    u8 temp;
	
	Coordinate gpsdata;
	
	u8 fall;     // 老人跌倒 状态
	u8 hrSta;
	u8 xueyangSta;
	u8 tempSta;

    uint32_t stepnum;   // 步数
}appData;

extern appData g_appdata;





// 图片数组
extern const u8 oldmen[];
extern const u8 oldmen_fall[];
extern const u8 SETUP[];
extern const u8 HEARTBEAT[];


#endif






