#ifndef _SMS4G_H
#define _SMS4G_H

#include "sys.h"


// 状态枚举定义
typedef enum {
    AT_IDLE = 0,            // 空闲等待状态
    AT_INIT,                // 初始化状态
    AT_HANDSHAKE,           // AT握手检测
    AT_ECHO_OFF,            // 关闭回显
    AT_CSMS_SET,            // 设置短信服务
    AT_TEXT_MODE,           // 文本模式设置
    AT_ENCODING_SET,        // 编码设置
    AT_PHONE_NUMBER,        // 准备手机号
    AT_CONTENT_PROMPT,      // 等待内容发送提示
    AT_SEND_CONTENT,        // 发送短信内容
    AT_SEND_TERMINATOR,     // 发送结束符
    AT_CONFIRMATION,        // 发送确认
    AT_COMPLETE,            // 完成状态
    AT_ERROR                // 错误状态
} SmsState;


// 全局状态机结构体
typedef struct {
    SmsState current_state; // 当前状态
    uint8_t retry_count;    // 当前重试次数
    uint32_t last_send;     // 最后发送时间戳
    char phone_num[16];     // 目标手机号
    char* content_ptr;      // 短信内容指针
    uint16_t content_len;    // 短信长度
} SmsStateMachine;


extern void send_sms_alarm(void);
extern void sms_fsm_handler(void);


#endif

