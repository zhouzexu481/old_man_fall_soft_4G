#ifndef _MXA30102_H
#define _MXA30102_H

#include "sys.h"

#define MAX30102_WR_address 0xAE
#define MAX30102_READ_ADDR 0xAF

#define INTERRUPT_STATUS1 0X00
#define INTERRUPT_STATUS2 0X01
#define INTERRUPT_ENABLE1 0X02
#define INTERRUPT_ENABLE2 0X03

#define FIFO_WR_POINTER 0X04
#define FIFO_OV_COUNTER 0X05
#define FIFO_RD_POINTER 0X06
#define FIFO_DATA 0X07

#define FIFO_CONFIGURATION 0X08
#define MODE_CONFIGURATION 0X09
#define SPO2_CONFIGURATION 0X0A
#define LED1_PULSE_AMPLITUDE 0X0C
#define LED2_PULSE_AMPLITUDE 0X0D
#define REG_PILOT_PA 0x10
#define MULTILED1_MODE 0X11
#define MULTILED2_MODE 0X12

#define TEMPERATURE_INTEGER 0X1F
#define TEMPERATURE_FRACTION 0X20
#define TEMPERATURE_CONFIG 0X21

#define VERSION_ID 0XFE
#define PART_ID 0XFF


typedef struct max30102
{
    uint16_t hr;
    float sqo2;

    uint8_t swing;
} max30102;


uint8_t max30102_init(void);
void max30102_fifo_read(float *data);
uint16_t max30102_getHeartRate(float *input_data,uint16_t cache_nums);
float max30102_getSpO2(float *ir_input_data,float *red_input_data,uint16_t cache_nums);
void ir_max30102_fir(float *input,float *output);
void red_max30102_fir(float *input,float *output);
void max30102_clear_status(void);
void heart_data_get(void);
int8_t get_blood_data(void *buf, uint32_t len);
uint8_t get_sw_status(void);


#define READ_MAX30102   PAin(0)

extern uint8_t max30102_int_flag;  		//????


#endif
