#ifndef __GPS_H__
#define __GPS_H__

#include "sys.h"

//定义数组长度
#define UTCTime_Length 11
#define data_length 8
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2

typedef struct gpsInfo
{
	char UTCTime[UTCTime_Length];		//UTC时间
	char szDATA[data_length];		//定位信息是否有效
	char latitude[latitude_Length];		//纬度
	char N_S[N_S_Length];		//N/S
	char longitude[longitude_Length];		//经度
	char E_W[E_W_Length];		//E/W
	char isUsefull[2];		//定位信息是否有效

} _gpsInfo;



extern _gpsInfo gpsdata;

void gnrmc_deal(char *cmdline, int cmd_len);
void gps_uart_init(void);


#endif
