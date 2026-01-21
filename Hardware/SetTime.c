#include "stm32f10x.h"                  // Device header

#include "MyRTC.h"

#include "OLED.h"
#include "Key.h"


/*--------------------[S] 界面设计 [S]--------------------*/

//日期时间设置第一页UI
void Show_SetTime_FirstUI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_Printf(0, 16, OLED_8X16, "年:%4d", MyRTC_Time[0]);
	OLED_Printf(0, 32, OLED_8X16, "月:%2d", MyRTC_Time[1]);
	OLED_Printf(0, 48, OLED_8X16, "日:%2d", MyRTC_Time[2]);
}

//日期时间设置第二页UI
void Show_SetTime_SecondUI(void)
{
	OLED_Printf(0, 0, OLED_8X16, "时:%02d", MyRTC_Time[3]);
	OLED_Printf(0, 16, OLED_8X16, "分:%02d", MyRTC_Time[4]);
	OLED_Printf(0, 32, OLED_8X16, "秒:%02d", MyRTC_Time[5]);
}
/*--------------------[E] 界面设计 [E]--------------------*/


/*--------------------[S] RTC数据更改 [S]--------------------*/

//参数i为MyRTC_time[]数值的索引值；flag=1为加1，flag=0为减1
void Change_RTC_Time(uint8_t i, uint8_t flag)
{
	if(flag == 1){MyRTC_Time[i]++;}
	else {MyRTC_Time[i]--;}
	MyRTC_SetTime();
}

int SetYear(void)
{
	while(1)
	{
		//上键，数值加1
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			Change_RTC_Time(0, 1);
		}
		//下键，数值减1
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			Change_RTC_Time(0, 0);
		}
		//确认键，退出
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			return 0;
		}
		
		Show_SetTime_FirstUI();
		OLED_ReverseArea(24, 16, 32, 16);
		OLED_Update();
	}
}

int SetMonth(void)
{
	while(1)
	{
		//上键，数值加1
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			Change_RTC_Time(1, 1);
			if (MyRTC_Time[1] >= 13){MyRTC_Time[1] = 1;MyRTC_SetTime();}
		}
		//下键，数值减1
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			Change_RTC_Time(1, 0);
			if (MyRTC_Time[1] <= 0){MyRTC_Time[1] = 12;MyRTC_SetTime();}
		}
		//确认键，保存并退出
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			return 0;
		}
		
		Show_SetTime_FirstUI();
		OLED_ReverseArea(24, 32, 16, 16);
		OLED_Update();
	}
}

int SetDay(void)
{
	while(1)
	{
		//上键，数值加1
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			Change_RTC_Time(2, 1);
			//不用额外判断一个月有几天，RTC函数自带判定
			if (MyRTC_Time[2] >= 32){MyRTC_Time[2] = 1;MyRTC_SetTime();}
		}
		//下键，数值减1
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			Change_RTC_Time(2, 0);
			if (MyRTC_Time[2] <= 0){MyRTC_Time[2] = 31;MyRTC_SetTime();}
		}
		//确认键，保存并退出
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			return 0;
		}
		
		Show_SetTime_FirstUI();
		OLED_ReverseArea(24, 48, 16, 16);
		OLED_Update();
	}
}

int SetHour(void)
{
	while(1)
	{
		//上键，数值加1
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			Change_RTC_Time(3, 1);
			//不用额外判断一个月有几天，RTC函数自带判定
			if (MyRTC_Time[3] >= 24){MyRTC_Time[3] = 0;MyRTC_SetTime();}
		}
		//下键，数值减1
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			Change_RTC_Time(3, 0);
			if (MyRTC_Time[3] < 0){MyRTC_Time[3] = 23;MyRTC_SetTime();}
		}
		//确认键，保存并退出
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			return 0;
		}
		
		Show_SetTime_SecondUI();
		OLED_ReverseArea(24, 0, 16, 16);
		OLED_Update();
	}
}

int SetMin(void)
{
	while(1)
	{
		//上键，数值加1
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			Change_RTC_Time(4, 1);
			//不用额外判断一个月有几天，RTC函数自带判定
			if (MyRTC_Time[4] >= 60){MyRTC_Time[4] = 0;MyRTC_SetTime();}
		}
		//下键，数值减1
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			Change_RTC_Time(4, 0);
			if (MyRTC_Time[4] < 0){MyRTC_Time[4] = 59;MyRTC_SetTime();}
		}
		//确认键，保存并退出
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			return 0;
		}
		
		Show_SetTime_SecondUI();
		OLED_ReverseArea(24, 16, 16, 16);
		OLED_Update();
	}
}

int SetSec(void)
{
		while(1)
	{
		//上键，数值加1
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			Change_RTC_Time(5, 1);
			//不用额外判断一个月有几天，RTC函数自带判定
			if (MyRTC_Time[5] >= 60){MyRTC_Time[5] = 0;MyRTC_SetTime();}
		}
		//下键，数值减1
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			Change_RTC_Time(5, 0);
			if (MyRTC_Time[5] < 0){MyRTC_Time[5] = 59;MyRTC_SetTime();}
		}
		//确认键，保存并退出
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			return 0;
		}
		
		Show_SetTime_SecondUI();
		OLED_ReverseArea(24, 32, 16, 16);
		OLED_Update();
	}
}

/*--------------------[E] RTC数据更改 [E]--------------------*/


/*--------------------[S] 模块交互代码 [S]--------------------*/

//日期时间设置界面选项标志位
uint8_t set_time_flag = 1;

uint8_t SetTime(void)
{
	while(1)
	{
		//存储确认键被按下时set_time_flag的值的临时变量，默认为无效值0
		uint8_t set_time_flag_temp = 0;
		
		//上键
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			set_time_flag --;
			if (set_time_flag <= 0) set_time_flag = 7;
		}
		//下键
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			set_time_flag ++;
			if (set_time_flag >= 8) set_time_flag = 1;
		}
		//确认键
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			//清屏OLED，准备跳转
			OLED_Clear();
			OLED_Update();
			
			set_time_flag_temp = set_time_flag;
		}
		
		//返回上一级菜单
		if (set_time_flag_temp == 1){return 0;}
		//"年"选项
		else if (set_time_flag_temp == 2){SetYear();}
		//"月"选项
		else if (set_time_flag_temp == 3){SetMonth();}
		//"日"选项
		else if (set_time_flag_temp == 4){SetDay();}
		//"时"选项
		else if (set_time_flag_temp == 5){SetHour();}
		//"分"选项
		else if (set_time_flag_temp == 6){SetMin();}
		//"秒"选项
		else if (set_time_flag_temp == 7){SetSec();}
		
		switch(set_time_flag)
		{
			//光标在第一页第一行"回车"键
			case 1:
			{
				OLED_Clear();
				Show_SetTime_FirstUI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在第一页第二行"年"选项
			case 2:
			{
				OLED_Clear();
				Show_SetTime_FirstUI();
				OLED_ReverseArea(0, 16, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在第一页第三行"月"选项
			case 3:
			{
				OLED_Clear();
				Show_SetTime_FirstUI();
				OLED_ReverseArea(0, 32, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在第一页第四行"日"选项
			case 4:
			{
				OLED_Clear();
				Show_SetTime_FirstUI();
				OLED_ReverseArea(0, 48, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在第二页第二行"时"选项
			case 5:
			{
				OLED_Clear();
				Show_SetTime_SecondUI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在第二页第二行"分"选项
			case 6:
			{
				OLED_Clear();
				Show_SetTime_SecondUI();
				OLED_ReverseArea(0, 16, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在第二页第二行"秒"选项
			case 7:
			{
				OLED_Clear();
				Show_SetTime_SecondUI();
				OLED_ReverseArea(0, 32, 16, 16);
				OLED_Update();
				
				break;
			}
			
			
		}
	}
}
/*--------------------[E] 模块交互代码 [E]--------------------*/
