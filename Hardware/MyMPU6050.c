#include "stm32f10x.h"

#include "OLED.h"
#include "Key.h"
#include "Delay.h"

#include "MPU6050.h"
#include "MahonyAHRS.h"
#include <math.h>

#define pi		3.14159265f;

/*--------------------[S] 界面设计 [S]--------------------*/

float Pitch, Roll, Yaw;
//欧拉角系数
float roll_k = 1.0f, pitch_k = 90.0f /
	85.0f, yaw_k = 90.0f / 5.0f;

void Show_MPU6050_UI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_Printf(0, 16, OLED_8X16, "Roll: %.2f", Roll  * roll_k);
	OLED_Printf(0, 32, OLED_8X16, "Pitch:%.2f", Pitch * pitch_k);
	OLED_Printf(0, 48, OLED_8X16, "Yaw:  %.2f", Yaw   * yaw_k);
}

/*--------------------[E] 界面设计 [E]--------------------*/


/*--------------------[S] 四元数转换 [S]--------------------*/

extern volatile float q0, q1, q2, q3;

void quat2euler(float q0,float q1,float q2,float q3,
	float* phi, float* theta, float* psi)
{
    float R[3][3];
    R[0][0] = 1 - 2 * (q2 * q2 + q3 * q3);
    R[0][1] = 2 * (q1 * q2 - q0 * q3);
    R[0][2] = 2 * (q0 * q2 + q1 * q3);
    R[1][0] = 2 * (q1 * q2 + q0 * q3);
    R[1][1] = 1 - 2 * (q1 * q1 + q3 * q3);
    R[1][2] = 2 * (q2 * q3 - q0 * q1);
    R[2][0] = 2 * (q1 * q3 - q0 * q2);
    R[2][1] = 2 * (q0 * q1 + q2 * q3);
    R[2][2] = 1 - 2 * (q1 * q1 + q2 * q2);

    *phi   = atan2( R[2][1], R[2][2]);
    *theta = asin (-R[2][0]);
    *psi   = atan2( R[1][0], R[0][0]);
	
	*phi   = *phi   * 180.0f / pi;
    *theta = *theta * 180.0f / pi;
    *psi   = *psi   * 180.0f / pi;
}

/*--------------------[E] 四元数转换 [E]--------------------*/


/*--------------------[S] 姿态解算 [S]--------------------*/

//MPU6050测得的三轴加速度和角速度
int16_t AX, AY, AZ, GX, GY, GZ;
//标准单位计算得的三轴加速度和角速度
float ax, ay, az, gx, gy, gz;

//零飘校准计数
uint8_t cali_count = 0;
//零飘校准标志位
uint8_t cali_flag = 0;
//零飘校准平均值
float ax_bias, ay_bias, az_bias;
float gx_bias, gy_bias, gz_bias;

void MPU6050_Calculation(void)
{
	MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
		
	if (cali_flag == 0) 
	{
		cali_count++;
		
		gx_bias += GX / 16.4f * (3.14159f / 180.0f);
		gy_bias += GY / 16.4f * (3.14159f / 180.0f);
		gz_bias += GZ / 16.4f * (3.14159f / 180.0f);
	
		if (cali_count >= 100) 
		{
			gx_bias = gx_bias / 100.0f;
			gy_bias = gy_bias / 100.0f;
			gz_bias = gz_bias / 100.0f;
			
			cali_flag = 1;
		}
		
		return ;
	}
	
	ax = AX / 2048.0f;
	ay = AY / 2048.0f;
	az = AZ / 2048.0f;
	
	gx = (GX / 16.4f) * (3.14159f / 180.0f) - gx_bias;
	gy = (GY / 16.4f) * (3.14159f / 180.0f) - gy_bias;
	gz = (GZ / 16.4f) * (3.14159f / 180.0f) - gz_bias;
	
	MahonyAHRSupdateIMU(gx, gy, gz, ax, ay, az);
	
	quat2euler(q0, q1, q2, q3, &Roll, &Pitch, &Yaw); 
	
	Delay_ms(2);
}

/*--------------------[E] 姿态解算 [E]--------------------*/

//控制光标在MPU6050界面移动的函数
int MPU6050(void)
{
	while(1)
	{
		if(Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		
		OLED_Clear();
		MPU6050_Calculation();
		Show_MPU6050_UI();
		OLED_ReverseArea(0,0,16,16);
		OLED_Update();
	}
}
