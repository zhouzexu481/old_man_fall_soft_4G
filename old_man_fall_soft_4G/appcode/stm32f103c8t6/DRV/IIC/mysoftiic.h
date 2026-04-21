/*
 * @FilePath: i2c.h
 * @Author: thinker
 * @Email: wangyun@appotronics.cn
 * @Description: I2C hal driver interface
 * @version: v1.0.0
 * @Date: 2023-03-26 21:19:27
 * @LastEditors: wangyun@appotronics.cn
 * @LastEditTime: 2023-03-26 22:17:16
 * @Copyright: Copyright (c) 2023
 */
#ifndef __MYSOFTIIC_H__
#define __MYSOFTIIC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "sys.h"

#define HAL_I2C_SLV_ADDR    (0x66)
#define HAL_SOFT_I2C_DELAY  (10)

#define I2C_ADDR_SIZE_8BIT  (1)
#define I2C_ADDR_SIZE_16BIT (2)

#define I2C_TX_BUFF_LEN     (64)

typedef void (*iic_rx_cb_t)(uint8_t );

typedef enum
{
    IIC_SOFT_NUM0,
    IIC_SOFT_NUM1,
    IIC_NUM_MAX,
} ae_iic_num;

typedef struct I2cGpio
{
    int (*scl_0)(ae_iic_num num);         
    int (*scl_1)(ae_iic_num num);         
    int (*sda_0)(ae_iic_num num);         
    int (*sda_1)(ae_iic_num num);         
    int (*sda_read)(ae_iic_num num);      
    void (*delay)(ae_iic_num num);    
    void (*init)(ae_iic_num num);        
    
    int (*start)(ae_iic_num num);                     
    int (*stop)(ae_iic_num num);                      
    int (*ack)(ae_iic_num num);                       
    int (*nack)(ae_iic_num num);                      
    int (*wait_ack)(ae_iic_num num);                  
    int (*send_byte)(ae_iic_num num, uint8_t byte);   
    uint8_t (*read_byte)(ae_iic_num num);             

    int (*reset)(ae_iic_num num);
    int (*transmit)(ae_iic_num num, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
    int (*receive)(ae_iic_num num, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
    int (*mem_write)(ae_iic_num num, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
    int (*mem_read)(ae_iic_num num, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
}as_iic_api;

typedef struct
{
    uint32_t sda_port;
    uint32_t sda_pin;
    uint32_t scl_port;
    uint32_t scl_pin;
    struct{
        uint8_t buff[I2C_TX_BUFF_LEN];
        uint16_t len;
        uint16_t r_idx;
    }tx;
    as_iic_api api; 
} as_iic_data;



void HAL_SOFT_I2C_Init(ae_iic_num num, uint32_t sda_port, uint32_t sda_pin, uint32_t scl_port, uint32_t scl_pin);
void HAL_SOFT_IIC_GPIO_Init(ae_iic_num num);
int HAL_SOFT_I2C_Transmit(ae_iic_num num, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
int HAL_SOFT_I2C_Receive(ae_iic_num num, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
int HAL_SOFT_I2C_MemWrite(ae_iic_num num, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
int HAL_SOFT_I2C_MemRead(ae_iic_num num, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);

#ifdef __cplusplus
}
#endif
#endif /* __I2C_H__ */
