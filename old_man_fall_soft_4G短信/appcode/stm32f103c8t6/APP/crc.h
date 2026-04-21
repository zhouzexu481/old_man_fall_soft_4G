/*
 * @File: crc.h
 * @Author: xusongbin@appotronics.cn
 * @Descripttion: Standard CRC interface
 * @version: v1.0.0
 * @Date: 2021-06-21 15:22:29
 * @LastEditors: xusongbin@appotronics.cn
 * @LastEditTime: 2021-09-13 09:51:32
 * @Copyright: Copyright (c) 2020
 */

#ifndef __CRC_H
#define __CRC_H

#include <stdint.h>

uint8_t crc8_maxim(uint8_t *buf, uint16_t len);
uint32_t crc32_getvalue(uint32_t crc, uint8_t * buffer, uint32_t size);

#endif

