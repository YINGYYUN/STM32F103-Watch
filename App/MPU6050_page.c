/*******************************************************************************
[菜单]MPU6050界面
*******************************************************************************/


#include "stm32f10x.h"

#include "Delay.h"
#include "common_headfile.h"
#include <math.h>


/**********************************************************/
/*[S] 界面样式 [S]----------------------------------------*/
/**********************************************************/

void Show_MPU6050_FirstUI(void)
{
	OLED_ShowImage(  0,  0, 16, 16, Return);
	OLED_ShowString( 39, 0, "[校准]", OLED_8X16);
	OLED_ShowString(115, 0, "R", OLED_8X16);
	OLED_ShowString(  0, 16, "Roll :", OLED_8X16);
	OLED_ShowString(  0, 32, "Pitch:", OLED_8X16);
	OLED_ShowString(  0, 48, "Yaw  :", OLED_8X16);
}

void Show_MPU6050_SecondUI(void)
{
	OLED_ShowImage(  0,  0, 16, 16, Return);
	OLED_ShowString( 39,  0, "[校准]", OLED_8X16);
	OLED_ShowString(115, 0, "C", OLED_8X16);
	OLED_ShowString(  0, 16, "ax:", OLED_6X8);
	OLED_ShowString(  0, 24, "ay:", OLED_6X8);
	OLED_ShowString(  0, 32, "az:", OLED_6X8);
	OLED_ShowString(  0, 40, "gx:", OLED_6X8);
	OLED_ShowString(  0, 48, "gy:", OLED_6X8);
	OLED_ShowString(  0, 56, "gz:", OLED_6X8);
}
/**********************************************************/
/*----------------------------------------[E] 界面样式 [E]*/
/**********************************************************/


/**********************************************************/
/*[S] 界面逻辑 [S]----------------------------------------*/
/**********************************************************/

// 控制光标在MPU6050界面移动的函数
int MPU6050(void)
{
	// MPU6050界面选项标志位
	uint8_t mpu6050_flag = 1;
	
	OLED_Clear();
	Show_MPU6050_FirstUI();
	OLED_ReverseArea(  0,  0, 16, 16);	
	OLED_Update();
	
	// 时间参考值重置
	Time_Count1 = 0;
	Time_Count2 = 0;
	
	uint8_t refresh = 1;			// UI刷新标志位
	uint8_t page = 0;				// 页面
	uint8_t cal_progress_bar = 4;	// 零飘校准进度条
	
	while(1)
	{
		// 存储确认键被按下时mpu6050_flag的值的临时变量，默认为无效值0
		uint8_t mpu6050_flag_temp = 0;
		
		/* 按键处理 */
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))// 上键
		{
			mpu6050_flag --;
			if (mpu6050_flag < 1)mpu6050_flag = 3;
			refresh = 1;
		}
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))// 下键
		{
			mpu6050_flag ++;
			if (mpu6050_flag > 3)mpu6050_flag = 1;
			refresh = 1;
		}
		else if(Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))// 确认键
		{
			mpu6050_flag_temp = mpu6050_flag;
		}
		
		/* 功能跳转 */
		// 返回上一级菜单
		if (mpu6050_flag_temp == 1){OLED_Clear();OLED_Update();return 0;}
		// 进行校准
		else if (mpu6050_flag_temp == 2)
		{
			OLED_ShowString( 39,  0, "校准中", OLED_8X16);
			OLED_Update();
	
			IMU_Gyro_Calib_Start(&gyro_cal);
			cal_progress_bar = 2;

            // 半阻塞式零飘校准
            while(1)
            {
				
                if (IMU_Gyro_Calib_Check(&gyro_cal) == GYRO_CALIB_STATE_DONE)  // 零飘校准完成
                {
                    break;  // 结束零飘校准
                }                                                             
                if(Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE)) // 强制零飘校准退出
                {
                    break;  // 中止零飘校准
                }        
				
				if (gyro_cal.calib_count >= GYRO_CALIB_TARGET_SAMPLES * cal_progress_bar / 100.0)
				{
					OLED_ShowString( 39,  0, "校准中", OLED_8X16);
					OLED_ReverseArea( 39, 0, cal_progress_bar/2, 16);
					OLED_UpdateArea( 39, 0, 48, 16);
					cal_progress_bar += 2;
				}
            }
			
			refresh = 1;
		}
		// 解算值R/原始值C 显示切换
		else if (mpu6050_flag_temp == 3)
		{
			page = 1 - page;
			refresh = 1;
		}
		 
		/* 姿态解算 */
		if (IMU_D_and_A_En == 1){IMU_Update_Analysis();}
		
		/* 显示更新 */
		if (Time_Count1 >= 100)	// 1ms * 100 的显示周期
		{
			Time_Count1 = 0;
			if (page == 0)
			{
				if (refresh)
				{
					OLED_Clear();
					Show_MPU6050_FirstUI();
					switch(mpu6050_flag)
					{
						// 光标在"回车"键
						case 1:
							OLED_ReverseArea(  0,  0, 16, 16);						
							break;
						
						// 光标在"校准"键
						case 2:
							OLED_ReverseArea( 39,  0, 48, 16);									
							break;
						// 光标在"R/C"键（解算值/原始值 显示切换）
						case 3:
							OLED_ReverseArea(111,  0, 16, 16);					
							break;
					}
					OLED_Update();
					refresh = 0;
				}
				OLED_Printf( 48, 16, OLED_8X16, "%.1f ", Roll_Result);
				OLED_Printf( 48, 32, OLED_8X16, "%.1f ", Pitch_Result);
				OLED_Printf( 48, 48, OLED_8X16, "%.1f ", Yaw_Result);
				OLED_Update();
			}
			else if (page == 1)
			{
				if (refresh)
				{
					OLED_Clear();
					Show_MPU6050_SecondUI();
					switch(mpu6050_flag)
					{
						// 光标在"回车"键
						case 1:
							OLED_ReverseArea(  0,  0, 16, 16);						
							break;
						
						// 光标在"校准"键
						case 2:
							OLED_ReverseArea( 39,  0, 48, 16);									
							break;
						// 光标在"R/C"键（解算值/原始值）
						case 3:
							OLED_ReverseArea(111,  0, 16, 16);					
							break;
					}
					OLED_Printf( 64, 40, OLED_6X8, "(%d)  ", gyro_cal.offset_x);
					OLED_Printf( 64, 48, OLED_6X8, "(%d)  ", gyro_cal.offset_y);
					OLED_Printf( 64, 56, OLED_6X8, "(%d)  ", gyro_cal.offset_z);
					OLED_Update();
					refresh = 0;
				}
				int16_t gx, gy, gz;
				IMU_Gyro_Apply(&gyro_cal, &gx, &gy, &gz);
				OLED_Printf( 18, 16, OLED_6X8, "%d  ", mpu6050_ax);
				OLED_Printf( 18, 24, OLED_6X8, "%d  ", mpu6050_ay);
				OLED_Printf( 18, 32, OLED_6X8, "%d  ", mpu6050_az);
				OLED_Printf( 18, 40, OLED_6X8, "%d  ", gx);
				OLED_Printf( 18, 48, OLED_6X8, "%d  ", gy);
				OLED_Printf( 18, 56, OLED_6X8, "%d  ", gz);
				OLED_Update();
			}
		}
	}
}
/**********************************************************/
/*----------------------------------------[E] 界面逻辑 [E]*/
/**********************************************************/
