#ifndef _SMS_H
#define _SMS_H

#include "sys.h"
#define MAX_SMS_LEN 1024

typedef enum sms_english
{
    SMS_STOP = 0,
    SMS_AT,
    SMS_AT_F,
    SMS_CSCS_GSM,
    SMS_CMGF_1,
    SMS_CMGS_PHONE,
    SMS_SEND_MESSAGE,
    SMS_SEND_MAX,
}sms_english_step;


typedef struct sms_control
{
    uint32_t sms_tick;  // 发送短信时间戳记录
    sms_english_step  sms_enstep; // 短信发送步骤
    char phone[12];         // 短信发送号码
    char smsbuf[128]; // 短信上报信息
    void (* sms_timeout_cb)(void);      // 短信超时回调函数
    void (*sms_english_send)(char *mesage);   // 短信英文发送
    uint32_t sms_timeout_tick;   // 短信超时时长设置
    uint32_t sms_english_timeout_tick[SMS_SEND_MAX];
    int8_t sms_timeout_num;     // 短信超时设置
}sms_control;


void my_sms_handle_rx(uint8_t *pData, uint32_t iLen);
void sms_gpio_on(void);
void sms_control_init(sms_control *msControl);

extern sms_control g_smsControl;

#endif

