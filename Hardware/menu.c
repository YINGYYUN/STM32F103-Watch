#include "stm32f10x.h"                  // Device header

#include "MyRTC.h"
#include "MPU6050.h"

#include "OLED.h"
#include "LED.h"
#include "Key.h"
#include "SetTime.h"					//[设置]日期时间设置模块
#include "SetBrightness.h"				//[设置]亮度设置模块
#include "StopWatch.h"					//[菜单]秒表模块
#include "MyMPU6050.h"					//[菜单]姿态解算模块

#include "Menu.h"

/*--------------------[S] 外设初始化 [S]--------------------*/

void Peripheral_Init(void)
{
	MyRTC_Init();
	LED_Init();
	Key_Init();
	MPU6050_Init();
}
/*--------------------[E] 外设初始化 [E]--------------------*/


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

//首页选项标志位
//菜单		1
//设置		2
uint8_t clkflag = 1;

int First_Page_Clock(void)
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


/*--------------------[S] 设置界面 [S]--------------------*/

//设置界面显示
void Show_SettingPage_UI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_ShowString(0, 16, "日期时间设置", OLED_8X16);
	OLED_ShowString(0, 32, "亮度设置", OLED_8X16);
}

//设置界面选项标志位
uint8_t setflag = 1;

int SettingPage(void)
{
	while(1)
	{
		//存储确认键被按下时setflag的值的临时变量，默认为无效值0
		uint8_t setflag_temp = 0;
		
		//上键
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			setflag --;
			if (setflag <= 0) setflag = 3;
		}
		//下键
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			setflag ++;
			if (setflag >= 4) setflag = 1;
		}
		//确认键
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			//清屏OLED，准备跳转
			OLED_Clear();
			OLED_Update();
			
			setflag_temp = setflag;
		}
		
		//返回上一级菜单
		if (setflag_temp == 1){return 0;}
		//进入日期时间设置界面
		else if (setflag_temp == 2){SetTime();}
		//进入屏幕亮度设置界面
		else if (setflag_temp == 3){SetBrightness();}
		
		switch(setflag)
		{
			//光标在第一行"回车"键
			case 1:
			{
				Show_SettingPage_UI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在第二行"日期时间设置"选项
			case 2:
			{
				Show_SettingPage_UI();
				OLED_ReverseArea(0, 16, 96, 16);
				OLED_Update();
				
				break;
			}
			//光标在第三行"亮度设置"选项
			case 3:
			{
				Show_SettingPage_UI();
				OLED_ReverseArea(0, 32, 64, 16);
				OLED_Update();
				
				break;
			}
			
			
		}
	}
}
/*--------------------[E] 设置界面 [E]--------------------*/


/*--------------------[S] 滑动菜单界面 [S]--------------------*/

//上次选择的选项
uint8_t pre_selection = 0;
//目标选项
uint8_t targrt_selection;
//上次选项的x坐标
int8_t x_pre = 48;
//图标移动速度
uint8_t Speed = 4;
//移动标志位;1开始，0停止
uint8_t move_flag;
//菜单选项数目(包括返回键)
#define NUM_SELECTION		7

//滑动菜单动画函数
void Menu_Animation(void)
{
	OLED_Clear();
	OLED_ShowImage(42, 10, 44, 44, Frame);
	//倒数第一个图标到第一个图标
	if (pre_selection == NUM_SELECTION -1 && targrt_selection == 0)
	{
		x_pre -= Speed;
		if (x_pre == 0)
		{
			pre_selection = 0;
			move_flag = 0;
			x_pre = 48;
		}
	}
	//第一个图标到倒数第一个图标
	else if (pre_selection == 0 && targrt_selection == NUM_SELECTION -1)
	{
		x_pre += Speed;
		if (x_pre >= 96)
		{
			pre_selection = NUM_SELECTION -1;
			move_flag = 0;
			x_pre = 48;
		}
	}
	else if (pre_selection < targrt_selection)
	{
		x_pre -= Speed;
		//前一个图标左移到左边缘
		if (x_pre == 0)
		{
			pre_selection ++;
			move_flag = 0;
			x_pre = 48;
		}
	}
	else if (pre_selection > targrt_selection)
	{
		x_pre += Speed;
		//前一个图标右移到右边缘
		if (x_pre >= 96)
		{
			pre_selection --;
			move_flag = 0;
			x_pre = 48;
		}
	}
	
	//前二的图标
	OLED_ShowImage(x_pre - 96, 16, 32, 32, Menu_Graph[(pre_selection - 2 + NUM_SELECTION) % NUM_SELECTION ]);
	//前一的图标
	OLED_ShowImage(x_pre - 48, 16, 32, 32, Menu_Graph[(pre_selection - 1 + NUM_SELECTION) % NUM_SELECTION ]);
	//当前的图标
	OLED_ShowImage(x_pre, 16, 32, 32, Menu_Graph[pre_selection]);
	//后一的图标
	OLED_ShowImage(x_pre + 48, 16, 32, 32, Menu_Graph[(pre_selection + 1 + NUM_SELECTION) % NUM_SELECTION ]);
	//后二的图标
	OLED_ShowImage(x_pre + 96, 16, 32, 32, Menu_Graph[(pre_selection + 2 + NUM_SELECTION) % NUM_SELECTION ]);

	OLED_Update();
}

void Set_Selection(uint8_t move_flag, uint8_t Pre_Selection, uint8_t Targrt_Selection)
{
	if (move_flag == 1)
	{
		pre_selection = Pre_Selection;
		targrt_selection = Targrt_Selection;	
	}
	//此函数外置，保证从功能中退出后能重新显示菜单界面
	Menu_Animation();
}

//转场函数（图标向下移动）
void MenuToFunction(void)
{
	for (uint8_t i = 0;i <= 6;i ++)
	{
		OLED_Clear();
		if (pre_selection >= 1)
		{
			OLED_ShowImage(x_pre - 48, 16 + 8*i, 32, 32, Menu_Graph[pre_selection - 1]);
		}
		OLED_ShowImage(x_pre, 16 + 8*i, 32, 32, Menu_Graph[pre_selection]);
		OLED_ShowImage(x_pre + 48, 16 + 8*i, 32, 32, Menu_Graph[pre_selection + 1]);
		
		OLED_Update();
	}
}

uint8_t menu_falg = 1;

int Menu(void)
{
	//move_flag=1;DirectFlag=0;使得进入菜单时处在退回键处
	move_flag = 1;
	//图标移动标志位；1移动到上一项，2移动到下一项，默认为无效值0
	uint8_t DirectFlag = 0;
	
	while(1)
	{
		//存储确认键被按下时menu_falg的值的临时变量，默认为无效值0
		uint8_t menu_falg_temp = 0;
		
		//上键
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			DirectFlag = 1;
			move_flag = 1;
			
			menu_falg --;
			if (menu_falg <= 0) menu_falg = NUM_SELECTION;
		}
		//下键
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			DirectFlag = 2;
			move_flag = 1;
			
			menu_falg ++;
			if (menu_falg >= NUM_SELECTION + 1) menu_falg = 1;
		}
		//确认键
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			//清屏OLED，准备跳转
			OLED_Clear();
			OLED_Update();
			
			menu_falg_temp = menu_falg;
		}
		
		//返回上一级菜单
		if (menu_falg_temp == 1){return 0;}
		//
		else if (menu_falg_temp == 2){MenuToFunction();StopWatch();}
		else if (menu_falg_temp == 3){}
		else if (menu_falg_temp == 4){MenuToFunction();MPU6050();}
		else if (menu_falg_temp == 5){}
		else if (menu_falg_temp == 6){}
		else if (menu_falg_temp == 7){}
		
		if (menu_falg == 1)
		{
			if (DirectFlag == 0)Set_Selection(move_flag, 0, 0);
			if (DirectFlag == 1)Set_Selection(move_flag, 1, 0);
			else if (DirectFlag == 2)Set_Selection(move_flag, NUM_SELECTION - 1, 0);
		}
		else if (menu_falg >= 1 && menu_falg <= 6)
		{
			if (DirectFlag == 1)Set_Selection(move_flag, menu_falg, menu_falg - 1);
			else if (DirectFlag == 2)Set_Selection(move_flag, menu_falg - 2, menu_falg - 1);
		}
		else if (menu_falg == NUM_SELECTION)
		{
			if (DirectFlag == 1)Set_Selection(move_flag, 0, NUM_SELECTION - 1);
			else if (DirectFlag == 2)Set_Selection(move_flag, NUM_SELECTION - 2, NUM_SELECTION - 1);
		}

	}
}
/*--------------------[E] 滑动菜单界面 [E]--------------------*/
