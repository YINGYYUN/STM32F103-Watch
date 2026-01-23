#include "stm32f10x.h"                  // Device header

#include "OLED.h"
#include "Key.h"

//开始计时标志位；1开始，0停止
uint8_t start_timing_flag = 0;

/*--------------------[S] 界面设计 [S]--------------------*/

uint8_t hour, min, sec;

void Show_StopWatch_UI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_Printf(32, 20, OLED_8X16, "%02d:%02d:%02d", hour, min, sec);
	if (start_timing_flag == 0)
	{
		OLED_ShowString(18, 44, "开始", OLED_8X16);
	}
	if (start_timing_flag == 1)
	{
		OLED_ShowString(18, 44, "停止", OLED_8X16);
	}
	OLED_ShowString(78, 44, "清除", OLED_8X16);
}

/*--------------------[E] 界面设计 [E]--------------------*/


/*--------------------[S] 读秒功能 [S]--------------------*/

void StopWatch_Tick(void)
{
	//分频用计数
	static uint16_t Count;
	Count ++;
	
	if (Count >= 1000)
	{
		Count = 0;
		if (start_timing_flag == 1)
		{
			sec ++;
			if (sec >= 60)
			{
				sec = 0;
				min ++;
				if (min >= 60)
				{
					min = 0;
					hour ++;
					if (hour > 99)
					{
						hour = 0;
					}
				}
			}
		}
	}
}

/*--------------------[E] 读秒功能 [E]--------------------*/

//秒表界面选项标志位
uint8_t stopwatch_flag = 1;

int StopWatch(void)
{
	while(1)
	{
		//存储确认键被按下时stopwatch_flag的值的临时变量，默认为无效值0
		uint8_t stopwatch_flag_temp = 0;
		
		//上键
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			stopwatch_flag --;
			if (stopwatch_flag <= 0) stopwatch_flag = 3;
		}
		//下键
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			stopwatch_flag ++;
			if (stopwatch_flag >= 4) stopwatch_flag = 1;
		}
		//确认键
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{		
			stopwatch_flag_temp = stopwatch_flag;
		}
		
		//返回上一级菜单
		if (stopwatch_flag_temp == 1){OLED_Clear();return 0;}
		//开始/停止计时
		else if (stopwatch_flag_temp == 2){start_timing_flag = 1 - start_timing_flag;}
		//计时清除
		else if (stopwatch_flag_temp == 3){start_timing_flag = 0;hour = min =sec = 0;}
		
		switch(stopwatch_flag)
		{
			//光标在"返回"键处
			case 1:
			{
				Show_StopWatch_UI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在"开始"/"停止"键处
			case 2:
			{
				Show_StopWatch_UI();			
				OLED_ReverseArea(18, 44, 32, 16);
				OLED_Update();
				
				break;
			}
			//光标在"清除"键处
			case 3:
			{
				Show_StopWatch_UI();		
				OLED_ReverseArea(78, 44, 32, 16);
				OLED_Update();
				
				break;
			}	
		}
	}
}
