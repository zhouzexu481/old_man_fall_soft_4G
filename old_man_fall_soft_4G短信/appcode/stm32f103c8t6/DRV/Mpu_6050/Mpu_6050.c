#include "Mpu_6050.h"
#include "mysoftiic.h"



void ByteWrite6050(uint8_t REG_Address,uint8_t REG_data);
uint8_t ByteRead6050(uint8_t REG_Address);
int Get6050Data(uint8_t REG_Address);
void InitMPU6050(void);
float Mpu6050AccelAngle(int8_t dir);
float Mpu6050GyroAngle(int8_t dir);

void mpu6050_get_accelangle(float *data)
{
	uint8_t regData[6];
	uint8_t i = 0;
	float result; 


	HAL_SOFT_I2C_MemRead(IIC_SOFT_NUM1, SlaveAddress, ACCEL_XOUT_H, 1, regData, 6);

	for (i = 0; i < 6; i += 2)
	{
		result = (float)((regData[i]<<8)+regData[i + 1]);
		data[i / 2] = (result + MPU6050_ZERO_ACCELL)/16384;
		data[i / 2] = data[i / 2]*1.2*180/3.14;
	}

}

//**************************************
// I2C写一个字节到MPU6050
//**************************************
void ByteWrite6050(uint8_t REG_Address,uint8_t REG_data)
{
    HAL_SOFT_I2C_MemWrite(IIC_SOFT_NUM1, SlaveAddress, REG_Address, 1, &REG_data, 1);
}

//**************************************
// I2C从MPU6050读取一个字节
//**************************************
uint8_t ByteRead6050(uint8_t REG_Address)
{
    uint8_t REG_data;

    HAL_SOFT_I2C_MemRead(IIC_SOFT_NUM1, SlaveAddress, REG_Address, 1, &REG_data, 1);
    
    return REG_data;
}

//**************************************
// 获取MPU6050两个字节的数据
//**************************************
int Get6050Data(uint8_t REG_Address)
{
    char H,L;
    H=ByteRead6050(REG_Address);
    L=ByteRead6050(REG_Address+1);
    return (H<<8)+L;   // 将高低字节合并
}

//**************************************
// 初始化MPU6050
//**************************************
void InitMPU6050(void)
{
    ByteWrite6050(PWR_MGMT_1, 0x00);  // 解除休眠模式
    ByteWrite6050(SMPLRT_DIV, 0x07);  // 设置采样率分频，典型值125Hz
    ByteWrite6050(CONFIG, 0x06);      // 设置低通滤波器，带宽5Hz
    ByteWrite6050(GYRO_CONFIG, 0x18); // 设置陀螺仪量程，±2000°/s
    ByteWrite6050(ACCEL_CONFIG, 0x01); // 设置加速度计量程，±2g
}

/*
**********************************************
** 功能：获取MPU6050加速度计角度
** 参数：dir - 要读取的轴
**       ACCEL_XOUT - X轴
**       ACCEL_YOUT - Y轴
**       ACCEL_ZOUT - Z轴
** 返回值：计算得到的角度值
** 说明：
**      ±2g量程下，加速度计灵敏度为16384 LSB/g
**      角度计算时，使用sinx≈x近似，deg = rad*180/3.14
**      由于x>=sinx，乘以1.2进行补偿
**********************************************
*/
float Mpu6050AccelAngle(int8_t dir)
{
    float accel_agle; // 加速度计角度
    float result; // 原始数据值
    result = (float)Get6050Data(dir); // 获取原始数据，转换为浮点数
    accel_agle = (result + MPU6050_ZERO_ACCELL)/16384; // 去除零点偏移，转换为g值
    accel_agle = accel_agle*1.2*180/3.14;     //转换为角度值
    
    return accel_agle; // 返回计算得到的角度
}

/*
**********************************************
** 功能：获取MPU6050陀螺仪角度
** 参数：dir - 要读取的轴
**       GYRO_XOUT - X轴
**       GYRO_YOUT - Y轴
**       GYRO_ZOUT - Z轴
** 返回值：计算得到的角度值
** 说明：
**      ±2000°/s量程下，陀螺仪灵敏度为16.4 LSB/(°/s)
**********************************************
*/
float Mpu6050GyroAngle(int8_t dir)
{
    float gyro_angle;
    gyro_angle = (float)Get6050Data(dir);   // 获取原始数据
    gyro_angle = -(gyro_angle + MPU6050_ZERO_GYRO)/16.4;    //去除零点偏移，转换为角度值
    
    return gyro_angle; // 返回计算得到的角度
}

