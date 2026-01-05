#include "stm32f10x.h"                  // Device header

#include "MyRTC.h"

#include "OLED.h"
#include "LED.h"
#include "Key.h"

/*外设初始化函数*/
void Peripheral_Init(void)
{
	MyRTC_Init();
	LED_Init();
	Key_Init();
}

/*--------------------[S] 首页时钟 [S]--------------------*/ 
void Show_Clock_UI(void)
{
	//读取RTC时间
	MyRTC_ReadTime();
	
	//显示年月日(屏幕左上角)
	OLED_Printf(0, 0, OLED_6X8, "%d-%d-%d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
	//显示时分秒(屏幕中央)
	OLED_Printf(16, 16, OLED_12X24 , "%02d:%02d:%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
	//显示"菜单"(屏幕左下角)16X16
	OLED_ShowString(0, 48, "菜单", OLED_8X16);
	//显示"设置"(屏幕右下角)16X16
	OLED_ShowString(96, 48, "设置", OLED_8X16);
}

//选项标志位
//菜单		1
//设置		2
uint8_t clkflag = 1;

uint8_t First_Page_Clock(void)
{
	while(1)
	{
		//上键
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			clkflag --;
			if (clkflag <= 0) clkflag = 2;
		}
		//下键
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			clkflag ++;
			if (clkflag >= 3) clkflag = 1;
		}
		//确认键
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			//清屏OLED，准备跳转
			OLED_Clear();
			OLED_Update();
			
			return clkflag;
		}
		
		switch(clkflag)
		{
			//光标在"菜单"
			case 1:
			{
				Show_Clock_UI();
				OLED_ReverseArea(0, 48, 32, 16);
				OLED_Update();
				
				break;
			}
			//光标在"设置"
			case 2:
			{
				Show_Clock_UI();
				OLED_ReverseArea(96, 48, 32, 16);
				OLED_Update();
				
				break;
			}
			
			
		}
	}
}

/*--------------------[E] 首页时钟 [E]--------------------*/ 
