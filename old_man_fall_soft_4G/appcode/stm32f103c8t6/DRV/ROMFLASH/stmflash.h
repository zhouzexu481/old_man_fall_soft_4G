#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include "sys.h"


#define STM32_FLASH_START_ADRESS     ((uint32_t)0x08000000) // STM32 FLASH的起始地址
#define FLASH_PAGE_SIZE              (1024)
#define STM32_FLASH_SIZE             (64 * 1024)
#define STM32_FLASH_END_ADDRESS      ((uint32_t)(STM32_FLASH_START_ADRESS + STM32_FLASH_SIZE))

#define FLASH_SAVE_ADDR  0X0800F800         // 用户参数保存起始地址


#define HR_MAX  200
#define HR_MIN  90

#define SQO2_MAX   100.0
#define SQO2_MIN   85.0

#define PONDING_MAX   50

#define SMS_ALARM_S   10

// #define PHONE   "19556632490" 

#define PHONE  "16675185744"


typedef struct
{
    uint8_t hr_min;
    uint8_t hr_max;

    uint8_t sqo2_min;
    uint8_t wendu_max;

    uint8_t phone[12];

    uint32_t crc32; // Data verification
} as_cfg_data;


void cfg_init(void);
void cfg_restore(void);


extern as_cfg_data cfg_data;

#endif

















