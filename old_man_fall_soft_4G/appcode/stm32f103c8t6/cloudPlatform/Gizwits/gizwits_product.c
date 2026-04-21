/**
************************************************************
* @file         gizwits_product.c
* @brief        Gizwits control protocol processing, and platform-related       hardware initialization 
* @author       Gizwits
* @date         2017-07-19
* @version      V03030000
* @copyright    Gizwits
* 
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值?|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/

#include <stdio.h>
#include <string.h>
#include "gizwits_product.h"
#include "common.h"
#include "app.h"
#include "gps.h"
#include "stmflash.h"

static uint32_t timerMsCount;
uint8_t aRxBuffer;

/** User area the current device state structure*/
dataPoint_t currentDataPoint;



/**@} */
/**@name Gizwits User Interface
* @{
*/

/**
* @brief Event handling interface

* Description:

* 1. Users can customize the changes in WiFi module status

* 2. Users can add data points in the function of event processing logic, such as calling the relevant hardware peripherals operating interface

* @param [in] info: event queue
* @param [in] data: protocol data
* @param [in] len: protocol data length
* @return NULL
* @ref gizwits_protocol.h
*/
int8_t gizwitsEventProcess(eventInfo_t *info, uint8_t *gizdata, uint32_t len)
{
    uint8_t i = 0;
    dataPoint_t *dataPointPtr = (dataPoint_t *)gizdata;
    moduleStatusInfo_t *wifiData = (moduleStatusInfo_t *)gizdata;
    protocolTime_t *ptime = (protocolTime_t *)gizdata;
    
#if MODULE_TYPE
    gprsInfo_t *gprsInfoData = (gprsInfo_t *)gizdata;
#else
    moduleInfo_t *ptModuleInfo = (moduleInfo_t *)gizdata;
#endif

    if((NULL == info) || (NULL == gizdata))
    {
        return -1;
    }

    for(i=0; i<info->num; i++)
    {
        switch(info->event[i])
        {

		case EVENT_TEMP_MAX:
            currentDataPoint.valueTEMP_MAX = dataPointPtr->valueTEMP_MAX;
            GIZWITS_LOG("Evt:EVENT_TEMP_MAX %4f\n",currentDataPoint.valueTEMP_MAX);
            //user handle
            cfg_data.wendu_max = currentDataPoint.valueTEMP_MAX;
            cfg_restore();
            break;
        case EVENT_HR_MAX:
            currentDataPoint.valueHR_MAX = dataPointPtr->valueHR_MAX;
            cfg_data.hr_max = (uint8_t)currentDataPoint.valueHR_MAX;
            cfg_restore();
            GIZWITS_LOG("Evt:EVENT_HR_MAX %d\n",currentDataPoint.valueHR_MAX);
            //user handle
            break;
        case EVENT_HR_MIN:
            currentDataPoint.valueHR_MIN = dataPointPtr->valueHR_MIN;
            cfg_data.hr_min = (uint8_t)currentDataPoint.valueHR_MIN;
            cfg_restore();
            GIZWITS_LOG("Evt:EVENT_HR_MIN %d\n",currentDataPoint.valueHR_MIN);
            //user handle
            break;
        case EVENT_SQO2_MIN:
            currentDataPoint.valueSQO2_MIN = dataPointPtr->valueSQO2_MIN;
            cfg_data.sqo2_min = currentDataPoint.valueSQO2_MIN;
            cfg_restore();
            GIZWITS_LOG("Evt:EVENT_SQO2_MIN %4f\n",currentDataPoint.valueSQO2_MIN);
            //user handle
            break;
        

        case EVENT_PHONE:
            GIZWITS_LOG("Evt: EVENT_PHONE\n");
            memcpy(currentDataPoint.valuePHONE,dataPointPtr->valuePHONE,sizeof(currentDataPoint.valuePHONE));
            memcpy(cfg_data.phone,currentDataPoint.valuePHONE,sizeof(cfg_data.phone));
            cfg_restore();
            //user handle
            break;           


        case WIFI_SOFTAP:
            break;
        case WIFI_AIRLINK:
            break;
        case WIFI_STATION:
            break;
        case WIFI_CON_ROUTER:
 
            break;
        case WIFI_DISCON_ROUTER:
 
            break;
        case WIFI_CON_M2M:
            if (g_appdata.wifiSta != 1)
            {
                // 表示刚才是断网状态，那么获取一下ntp
                // gizwitsGetNTP();
            }
            g_appdata.wifiSta = 1;
            break;
        case WIFI_DISCON_M2M:
            g_appdata.wifiSta = 0;
            break;
        case WIFI_RSSI:
            GIZWITS_LOG("RSSI %d\n", wifiData->rssi);
            break;
        case TRANSPARENT_DATA:
            GIZWITS_LOG("TRANSPARENT_DATA \n");
            //user handle , Fetch data from [data] , size is [len]
            break;
        case WIFI_NTP:
//            GIZWITS_LOG("WIFI_NTP : [%d-%d-%d %02d:%02d:%02d][%d] \n",ptime->year,ptime->month,ptime->day,ptime->hour,ptime->minute,ptime->second,ptime->ntp);
//            // 得到网络时间，设置NTC
//            RTC_Set(ptime->year, ptime->month, ptime->day, ptime->hour, ptime->minute, ptime->second);
            break;
        case MODULE_INFO:
            GIZWITS_LOG("MODULE INFO ...\n");
#if MODULE_TYPE
            GIZWITS_LOG("GPRS MODULE ...\n");
            //Format By gprsInfo_t
            GIZWITS_LOG("moduleType : [%d] \n",gprsInfoData->Type);
#else
            GIZWITS_LOG("WIF MODULE ...\n");
            //Format By moduleInfo_t
            GIZWITS_LOG("moduleType : [%d] \n",ptModuleInfo->moduleType);
#endif
        break;
        default:
            break;
        }
    }

    return 0;
}

/**
* User data acquisition

* Here users need to achieve in addition to data points other than the collection of data collection, can be self-defined acquisition frequency and design data filtering algorithm

* @param none
* @return none
*/
void userHandle(void)
{
    currentDataPoint.valuexintiao = g_appdata.HR;//Add Sensor Data Collection
    currentDataPoint.valuexueyang = g_appdata.SpO2;//Add Sensor Data Collection
    currentDataPoint.valuetiwen = g_appdata.temp;//Add Sensor Data Collection
    currentDataPoint.valuelat = g_appdata.gpsdata.gcj_lat;
	currentDataPoint.valuelon = g_appdata.gpsdata.gcj_lng;

    currentDataPoint.valueHR_MAX = cfg_data.hr_max;
    currentDataPoint.valueHR_MIN = cfg_data.hr_min;
    currentDataPoint.valueSQO2_MIN = cfg_data.sqo2_min;
    currentDataPoint.valueTEMP_MAX = cfg_data.wendu_max;
    currentDataPoint.valuetiwen = g_appdata.temp;
		currentDataPoint.valuefallSta = !g_appdata.fall;
		currentDataPoint.valuexinlvSta = g_appdata.hrSta;
	currentDataPoint.valuexueyangSta = g_appdata.xueyangSta;
	currentDataPoint.valuetempSta = g_appdata.tempSta;
    // currentDataPoint.valueair_wendu = g_appdata.air_temp;
    // currentDataPoint.valueair_shidu = g_appdata.air_huim;
    memcpy(currentDataPoint.valuePHONE, cfg_data.phone, 12);
}

/**
* Data point initialization function

* In the function to complete the initial user-related data
* @param none
* @return none
* @note The developer can add a data point state initialization value within this function
*/
void userInit(void)
{
    memset((uint8_t*)&currentDataPoint, 0, sizeof(dataPoint_t));
    
    /** Warning !!! DataPoint Variables Init , Must Within The Data Range **/ 
    /*
    currentDataPoint.valuexintiao = ;
    currentDataPoint.valueair_wendu = ;
    currentDataPoint.valueair_shidu = ;
    currentDataPoint.valueHR_MAX = ;
    currentDataPoint.valueHR_MIN = ;
    currentDataPoint.valueSQO2_MAX = ;
    currentDataPoint.valueSQO2_MIN = ;
    currentDataPoint.valueSMS_ALARM = ;
    currentDataPoint.valuexueyang = ;
    currentDataPoint.valuewendu = ;
    currentDataPoint.valuelat = ;
    currentDataPoint.valuelon = ;
    */

}


/**
* @brief Millisecond timing maintenance function, milliseconds increment, overflow to zero

* @param none
* @return none
*/
void gizTimerMs(void)
{
    timerMsCount++;
}

/**
* @brief Read millisecond count

* @param none
* @return millisecond count
*/
uint32_t gizGetTimerCount(void)
{
    return timerMsCount;
}

/**
* @brief MCU reset function

* @param none
* @return none
*/
void mcuRestart(void)
{
    __set_FAULTMASK(1);
//    HAL_NVIC_SystemReset();
}



/**
* @brief Serial port write operation, send data to WiFi module
*
* @param buf      : buf address
* @param len      : buf length
*
* @return : Return effective data length;-1锛宺eturn failure
*/
int32_t uartWrite(uint8_t *buf, uint32_t len)
{
		uint8_t crc[1] = {0x55};
    uint32_t i = 0;
	
    if(NULL == buf)
    {
        return -1;
    }

    for(i=0; i<len; i++)
    {
        // HAL_UART_Transmit_IT(&huart2, (uint8_t *)&buf[i], 1);
        while ((USART3->SR & 0x40) == 0)
            ;
        USART_SendData(USART3, buf[i]);
				// while (huart2.gState != HAL_UART_STATE_READY);//Loop until the end of transmission

        if(i >=2 && buf[i] == 0xFF)
        {
						// HAL_UART_Transmit_IT(&huart2, (uint8_t *)&crc, 1);
            while ((USART3->SR & 0x40) == 0)
                ;
            USART_SendData(USART3, crc[0]);
						// while (huart2.gState != HAL_UART_STATE_READY);//Loop until the end of transmission
        }
    }

#ifdef PROTOCOL_DEBUG
    GIZWITS_LOG("MCU2WiFi[%4d:%4d]: ", gizGetTimerCount(), len);
    for(i=0; i<len; i++)
    {
        GIZWITS_LOG("%02x ", buf[i]);

        if(i >=2 && buf[i] == 0xFF)
        {
            GIZWITS_LOG("%02x ", 0x55);
        }
    }
    GIZWITS_LOG("\n");
#endif
		
		return len;
}  
