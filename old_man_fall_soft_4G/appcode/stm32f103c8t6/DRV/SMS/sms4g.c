#include "sms4g.h"
#include "delay.h"
#include "usart.h"
#include "stmflash.h"
#include "app.h"
#include "string.h"
#include <stddef.h> // 基础定义（C标准库）

#include <wchar.h> // 添加这行解决 wcslen 和 swprintf 警告

// 增加提前声明
uint8_t check_response(const char *expected);
uint8_t check_timeout(void);
void handle_retry(SmsState prev_state);

SmsStateMachine g_sms_fsm = {0};
const uint8_t MAX_RETRY = 3;
const uint32_t CMD_TIMEOUT = 2000; // 2秒超时

static wchar_t final_str[128];
// 编码转换优化
static uint8_t ucs2_payload[512];

char rx_buf[128];

// GB2312到Unicode映射函数
wchar_t gb2312_to_unicode(unsigned char high, unsigned char low)
{
    // 核心汉字映射
    if (high == 0xC0 && low == 0xCF)
        return L'老'; // C0CF → 8001
    if (high == 0xC8 && low == 0xCB)
        return L'人'; // C8CB → 4EBA
    if (high == 0xBF && low == 0xC9)
        return L'可'; // BFC9 → 53EF
    if (high == 0xC4 && low == 0xDC)
        return L'能'; // C4DC → 80FD
    if (high == 0xB7 && low == 0xA2)
        return L'发'; // B7A2 → 53D1
    if (high == 0xC9 && low == 0xFA)
        return L'生'; // C9FA → 751F
    if (high == 0xCE && low == 0xA3)
        return L'危'; // CEA3 → 5371
    if (high == 0xCF && low == 0xD5)
        return L'险'; // CFD5 → 9669
    if (high == 0xD3 && low == 0xD0)
        return L'用'; // D3D0 → 7528
    if (high == 0xBB && low == 0xA7)
        return L'户'; // BBA7 → 6237
                      // 新增健康短信字符
    if (high == 0xCE && low == 0xD2)
        return L'我'; // CED2 → 6211
    if (high == 0xCF && low == 0xD2)
        return L'现'; // CFD2 → 73B0
    if (high == 0xD4 && low == 0xDA)
        return L'在'; // D4DA → 5728
    if (high == 0xBD && low == 0xA1)
        return L'健'; // BDA1 → 5065
    if (high == 0xBF && low == 0xB5)
        return L'康'; // BFB5 → 5EB7

    // 经度纬度相关
    if (high == 0xBE && low == 0xAD)
        return L'经'; // BEAD → 7ECF
    if (high == 0xB6 && low == 0xC8)
        return L'度'; // B6C8 → 5EA6
    if (high == 0xCE && low == 0xAC)
        return L'纬'; // CEAC → 7EBD

    // 医疗参数扩展
    if (high == 0xD0 && low == 0xC4)
        return L'心'; // 心率 D0C4 → 5FC3
    if (high == 0xC2 && low == 0xCA)
        return L'率'; // C2CA → 7387
    if (high == 0xD1 && low == 0xAA)
        return L'血'; // D1AA → 8840
    if (high == 0xD1 && low == 0xF4)
        return L'氧'; // D1F4 → 6C27
    if (high == 0xCC && low == 0xE5)
        return L'体'; // CCE5 → 4F53
    if (high == 0xCE && low == 0xC2)
        return L'温'; // CEC2 → 6E29

    // 括号处理
    if (high == 0xA3 && low == 0xA8)
        return L'（'; // A3A8 → FF08
    if (high == 0xA3 && low == 0xA9)
        return L'）'; // A3A9 → FF09

    return L'□'; // 未识别字符占位符
}

// 新增 UCS-2 编码转换函数（大端序）
void unicode_to_ucs2be(wchar_t *unicode_str, uint8_t *ucs2_buf)
{
    int i = 0;
    while (unicode_str[i] != L'\0')
    {
        ucs2_buf[2 * i] = (unicode_str[i] >> 8) & 0xFF; // 高字节在前
        ucs2_buf[2 * i + 1] = unicode_str[i] & 0xFF;    // 低字节在后
        i++;
    }
    ucs2_buf[2 * i] = 0x00; // 结束符
}

/**
 * @brief  发送下一条AT指令并更新状态机
 * @param  cmd    : 待发送的AT指令
 * @param  next_state : 下一个目标状态
 * @retval 发送是否成功
 */
uint8_t send_next_command(const char *cmd, SmsState next_state)
{
    USART1_Send_Data((uint8_t *)cmd, strlen(cmd));

    g_sms_fsm.current_state = next_state; // 推进状态
    g_sms_fsm.retry_count = 0;            // 重置重试计数器
    g_sms_fsm.last_send = systicks_get(); // 更新时间戳
    return 1;
}

/* 状态处理函数 */
void sms_fsm_handler(void)
{
    switch (g_sms_fsm.current_state)
    {

    case AT_IDLE:
        // 无操作，等待触发
        break;

    case AT_COMPLETE:
        // log_sms_result(STATUS_OK);
        g_sms_fsm.current_state = AT_IDLE; // 完成回到空闲态
        break;

    case AT_INIT:
    {

       
        USART1_Send_Data((uint8_t *)"AT\r\n", 4);

        g_sms_fsm.current_state = AT_HANDSHAKE;
        g_sms_fsm.last_send = systicks_get();
    }
    break;

    case AT_HANDSHAKE:
        if (check_response("OK"))
        {
            send_next_command("ATE0\r\n", AT_ECHO_OFF);
        }
        else if (check_timeout())
        {
            handle_retry(AT_INIT);
        }
        break;

    case AT_ECHO_OFF:
        if (check_response("OK"))
        {
            send_next_command("AT+CSMS=1\r\n", AT_CSMS_SET);
        }
        else if (check_timeout())
        {
            handle_retry(AT_ECHO_OFF);
        }
        break;

    case AT_CSMS_SET:
        if (check_response("OK"))
        {
            send_next_command("AT+CMGF=1\r\n", AT_TEXT_MODE);
        }
        else if (check_timeout())
        {
            handle_retry(AT_ECHO_OFF);
        }
        break;

    case AT_TEXT_MODE:
        if (check_response("OK"))
        {
            send_next_command("AT+CSMP=17,167,0,8\r\n", AT_ENCODING_SET);
        }
        else if (check_timeout())
        {
            handle_retry(AT_ECHO_OFF);
        }
        break;

    case AT_ENCODING_SET:
        if (check_response("OK"))
        {
            g_sms_fsm.current_state = AT_PHONE_NUMBER; // 推进状态
            g_sms_fsm.retry_count = 0;                 // 重置重试计数器
            g_sms_fsm.last_send = systicks_get();      // 更新时间戳
        }
        else if (check_timeout())
        {
            handle_retry(AT_ECHO_OFF);
        }
        break;

    case AT_PHONE_NUMBER:
    {
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"\r\n", g_sms_fsm.phone_num);
        if (USART1_Send_Data((uint8_t *)cmd, strlen(cmd)))
        {
            g_sms_fsm.current_state = AT_CONTENT_PROMPT;
            g_sms_fsm.last_send = systicks_get();
        }
    }
    break;

    case AT_CONTENT_PROMPT:
        if (check_response("> "))
        { // 等待模块提示符
            g_sms_fsm.current_state = AT_SEND_CONTENT;
            // prepare_sms_content(); // 准备编码内容
        }
        else if (check_timeout())
        {
            handle_retry(AT_PHONE_NUMBER);
            USART1_Send_Data((uint8_t *)"\x1A", 1); // 防止上一次一场失败
        }
        break;

    case AT_SEND_CONTENT:
        if (USART1_Send_Data((uint8_t *)g_sms_fsm.content_ptr, g_sms_fsm.content_len))
        {
            g_sms_fsm.current_state = AT_SEND_TERMINATOR;
            g_sms_fsm.last_send = systicks_get();
        }
        break;

    case AT_SEND_TERMINATOR:
        if (USART1_Send_Data((uint8_t *)"\x1A", 1))
        { // 发送Ctrl+Z
            g_sms_fsm.current_state = AT_CONFIRMATION;
            g_sms_fsm.last_send = systicks_get();
        }
        break;

    case AT_CONFIRMATION:
        if (check_response("+CMGS:"))
        { // 成功响应
            g_sms_fsm.current_state = AT_COMPLETE;
            g_sms_fsm.retry_count = 0;            // 重置重试计数器
            g_sms_fsm.last_send = systicks_get(); // 更新时间戳
        }
        else if (check_timeout())
        {
            handle_retry(AT_PHONE_NUMBER); // 失败回退到手机号阶段
        }
        break;

    case AT_ERROR:
        g_sms_fsm.current_state = AT_IDLE;
        g_sms_fsm.retry_count = 0;            // 重置重试计数器
        g_sms_fsm.last_send = systicks_get(); // 更新时间戳
        break;
    }
}


/* 检查响应（带缓冲区管理）*/
uint8_t check_response(const char *expected)
{
    static uint8_t atCmdBuf[128] = {0};
    static uint32_t t_i = 0;
    uint8_t rxLen = 0;
    uint8_t i = 0;

    rxLen = queue_read(&my_uart1_rx_Q, rx_buf, sizeof(rx_buf));

    if (rxLen <= 0)
    {
        return 0;
    }


    for (i = 0; i < rxLen; i++)
    {
        if (rx_buf[i] == '\r' || rx_buf[i] == '\n') // 回车是\r 为了兼容同时处理 \n
        {
           
            if (strstr(atCmdBuf, expected))
            {
                t_i = 0;
                return 1;
            }

            atCmdBuf[0] = 0;
            t_i = 0;

            // 如果下个字符是\n，跳过
            if (rx_buf[i + 1] == '\n')
            {
                i++;
            }
        }
        else if (t_i < (MAX_CMD_LEN - 1))
        {
            atCmdBuf[t_i++] = rx_buf[i];
            atCmdBuf[t_i] = '\0';
        }
    }

    return 0;
}

/* 超时检测 */
uint8_t check_timeout(void)
{
    return (systicks_get() - g_sms_fsm.last_send) > CMD_TIMEOUT;
}

/* 重试处理 */
void handle_retry(SmsState prev_state)
{
    if (++g_sms_fsm.retry_count >= MAX_RETRY)
    {
        g_sms_fsm.current_state = AT_ERROR;
        g_sms_fsm.retry_count = 0;
    }
    else
    {
        g_sms_fsm.current_state = prev_state;
        g_sms_fsm.last_send = systicks_get();
    }
}

// 增加触发接口
void sms_send_trigger(const char *num, const char *content, uint16_t len)
{
    if (g_sms_fsm.current_state == AT_IDLE)
    {
        memset(g_sms_fsm.phone_num, 0, sizeof(g_sms_fsm.phone_num));
        strncpy(g_sms_fsm.phone_num, num, strlen(num));
        g_sms_fsm.content_ptr = (char *)content;
        g_sms_fsm.content_len = len;
        g_sms_fsm.current_state = AT_INIT; // 触发发送流程
    }
}

// 优化后的短信发送函数
void send_sms_alarm(void)
{
    double lon = g_appdata.gpsdata.gcj_lng;
    double lat = g_appdata.gpsdata.gcj_lat;

    // 安全格式化（限制长度）
    swprintf(final_str, sizeof(final_str)/sizeof(wchar_t),
            L"用户可能发生危险 心率（%d）血氧（%.1f）体温（%d）经度（%.4f）纬度（%.4f）", 
            g_appdata.HR, 
            g_appdata.SpO2, 
            g_appdata.temp,
            lon, 
            lat);


    memset(ucs2_payload, 0, sizeof(ucs2_payload));

    // 转换为UCS-2大端格式
    unicode_to_ucs2be(final_str, ucs2_payload);

    uint16_t payload_len = wcslen(final_str) * 2; // UCS-2每个字符占2字节
    sms_send_trigger(cfg_data.phone, ucs2_payload, payload_len);
}
