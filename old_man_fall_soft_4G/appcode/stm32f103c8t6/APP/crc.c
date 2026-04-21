/*
 * @File: crc.c
 * @Author: xusongbin@appotronics.cn
 * @Descripttion: Standard CRC interface
 * @version: v1.0.0
 * @Date: 2021-06-21 15:22:29
 * @LastEditors: xusongbin@appotronics.cn
 * @LastEditTime: 2021-09-13 09:51:33
 * @Copyright: Copyright (c) 2020
 */

#include "crc.h"


/**
 * @function: crc8_maxim
 * @description: Calculates the CRC8 check value of the array
 * @param {uint8_t} *buf
 * @param {uint16_t} len
 * @return {*}
 * @record: 
 */
uint8_t crc8_maxim(uint8_t *buf, uint16_t len)
{
	uint8_t crc;
	uint8_t i;
	
	crc = 0;
	while(len--)
	{
		crc ^= *buf++;
		for(i = 0;i < 8;i++)
		{
			if(crc & 0x01)
			{
				crc = (crc >> 1) ^ 0x8c;
			}
			else
				crc >>= 1;
		}
	}
	return crc; 
}

/**
 * @function: crc32_getvalue
 * @description: Calculates the CRC32 check value of the array. crc in default 0xFFFFFFFF.
 * @param {uint32_t} crc
 * @param {uint8_t *} buffer
 * @param {uint32_t} size
 * @return {*}
 * @record: 
 */
uint32_t crc32_getvalue(uint32_t crc, uint8_t * buffer, uint32_t size)
{
	uint32_t i, crc_temp, mask;
	uint8_t byte, j;

	i = 0;
	crc_temp = crc;
	while (size != 0)
	{
		byte = buffer[i];            			// Get next byte.
		crc_temp = crc_temp ^ byte;
		for (j = 8; j > 0; j--)					// Do eight times.
		{
			mask = -(crc_temp & 1);
			crc_temp = (crc_temp >> 1) ^ (0xEDB88320 & mask);
		}
		i = i + 1;
		size--;
	}
	return crc_temp;
}


