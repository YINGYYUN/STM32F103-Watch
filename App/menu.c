/*******************************************************************************
[首页]+[菜单]+[设置]
*******************************************************************************/


#include "stm32f10x.h"                  // Device header
#include "MyRTC.h"
#include "common_headfile.h"

/**********************************************************/
/*[S] 外设初始化 [S]--------------------------------------*/
/**********************************************************/

void Peripheral_Init(void)
{
	MyRTC_Init();
	LED_Init();
	Key_Init();
	MPU6050_Init();
	AD_Init();
}
/**********************************************************/
/*--------------------------------------[E] 外设初始化 [E]*/
/**********************************************************/


/**********************************************************/
/*[S] 首页时钟界面 [S]------------------------------------*/
/**********************************************************/

// 使用滚动显示时间的开关
#define ROLL_CLOCK_ENABLE		1

uint16_t AD_Value = 0;
//float VBAT = 0.0f;
int Battery_Capacity = 0;
// 实际上ADC拿到的是 1KΩ(靠近VCC) 和 4.4KΩ(靠近GND) 串联电阻中4.4KΩ 拿到的电压
// 教程设计的4KΩ被将就为4.4KΩ
// ADC检测到的电池电压范围为 2.68 ~ 3.34V (对应电池的 3.3 ~ 4.1V)
// 数值上为 [0%]3049 ~ [100%]4096 (实际值4145超出量程，仍然认定4096也是100%)

/* 电池电量显示 */
void Show_Battery(void)
{
	static uint16_t last_AD = 0;		// 上次更新的AD值，滞回用
	static int last_capacity = 0;		// 上次显示的百分比，滞回用
	int sum = 0;
	for (int i = 0;i < 1000; i ++)
	{
		AD_Value = AD_GetValue();
		sum += AD_Value;
	}
	AD_Value = sum / 1000;

	/* 滞回：变化≥5个ADC计数(≈0.5%)才更新百分比 */
	int16_t delta = (int16_t)(AD_Value - last_AD);
	if (delta < 0) delta = -delta;
	if (delta >= 5)
	{
		last_AD = AD_Value;
		Battery_Capacity = (AD_Value - 3049)*100/1047;
		if (Battery_Capacity < 0)Battery_Capacity = 0;
		last_capacity = Battery_Capacity;
	}
	else
	{
		Battery_Capacity = last_capacity;	// 用缓存值绘制
	}
	
	
//	OLED_ShowNum( 64,  0 , AD_Value, 4, OLED_6X8);
//	OLED_Printf( 64,  8, OLED_6X8, "VBAT:%.2f", VBAT);
	OLED_ShowNum( 85,  1 , Battery_Capacity, 3, OLED_6X8);
	OLED_ShowChar(103,  1, '%', OLED_6X8);
	
	if (Battery_Capacity == 100){OLED_ShowImage(110,  0, 16, 16, Battery);}
	else if (10 <= Battery_Capacity && Battery_Capacity < 100)
	{
		OLED_ShowImage(110,  0, 16, 16, Battery);
		OLED_ClearArea((112 + Battery_Capacity/10),  2, (10 - Battery_Capacity/10), 6);
		OLED_ClearArea( 85,  1,  6,  8);
	}
	else
	{
		OLED_ShowImage(110,  0, 16, 16, Battery);
		OLED_ClearArea(112,  2, 10,  6);
		OLED_ClearArea( 85,  1, 12,  8);
	}

}

/* 界面样式*/
void Show_Clock_UI(uint8_t clkflag)
{
	// 显示电池电量
	Show_Battery();
	// 显示年月日(屏幕左上角)
	OLED_Printf(  0,  1, OLED_6X8, "%d-%d-%d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
	// 显示"菜单"(屏幕左下角)16X16
	OLED_ShowString(  0, 48, "菜单", OLED_8X16);
	// 显示"设置"(屏幕右下角)16X16
	OLED_ShowString( 96, 48, "设置", OLED_8X16);
	if (clkflag == 1){OLED_ReverseArea(  0, 48, 32, 16);}
	else {OLED_ReverseArea( 96, 48, 32, 16);}
	
	OLED_UpdateArea(  0, 48, 128, 16);
	OLED_UpdateArea(  0,  0, 60,  8);		// 日期区域, y=1
	OLED_UpdateArea( 85,  0, 41, 16);		// 电池区域, x:85~125 y:0~15
	
	// 读取RTC时间
	MyRTC_ReadTime();
		
	// 显示时分秒(屏幕中央)
#if ROLL_CLOCK_ENABLE 
	Roll_Clock();
#else
	OLED_Printf( 16, 16, OLED_12X24 , "%02d:%02d:%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
	OLED_UpdateArea( 16, 16, 96, 24);
#endif
	
	OLED_Clear();
}

// 首页选项标志位
// 菜单		1
// 设置		2
uint8_t clkflag = 1;

/* 交互界面*/
int First_Page_Clock(void)
{
	// 此处使得返回首页时，将先显示时间，后显示其他内容，再进行滚动时间判定
#if ROLL_CLOCK_ENABLE
	// 时间滚动显示初始化，和一次静态时间显示
	Roll_Clock_Init();
#endif	
	
	// 时间参考值重置
	Time_Count1 = 0;
	Time_Count2 = 0;
	
	uint8_t power_on = 1;	// 手表开关状态

	while(1)
	{
		/* 按键处理 */
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))// 上键 短按
		{
			clkflag --;
			if (clkflag < 1) clkflag = 2;
		}		
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))// 下键 短按
		{
			clkflag ++;
			if (clkflag > 2) clkflag = 1;
		}
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))// 确认键 短按
		{
			// 清屏OLED，准备跳转
			OLED_Clear();
			OLED_Update();
			
			// 返回[菜单]和[设置]中的选择(对象位于"main.c")
			return clkflag;
		}
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_LONG))// 确认键 长按
		{
			// 清屏OLED
			OLED_Clear();
			OLED_Update();
			power_on = 0;
			
			GPIO_ResetBits(GPIOB, GPIO_Pin_13);// 拉低CTL引脚（PB13),单片机关机
			GPIO_SetBits(GPIOB, GPIO_Pin_12);// 拉高BAT_ADC_EN引脚（PB12),ADC检测电路断开
		}
		
		
		/* 显示更新 */	
		if (power_on && Time_Count1 >= 80) // 1ms * 80 显示周期 （时间数字的滚动动画是直接阻塞占用的）
		{
			Time_Count1 = 0;
			Show_Clock_UI(clkflag);
		}
	}
}
/**********************************************************/
/*------------------------------------[E] 首页时钟界面 [E]*/
/**********************************************************/


/**********************************************************/
/*[S] 设置界面 [S]----------------------------------------*/
/**********************************************************/

/* 界面样式*/
void Show_SettingPage_UI(void)
{
	OLED_ShowImage(  0,  0, 16, 16, Return);
	OLED_ShowString(  0, 16, "日期时间设置", OLED_8X16);
	OLED_ShowString(  0, 32, "亮度设置", OLED_8X16);
}

// 设置界面选项标志位
uint8_t setflag = 1;

/* 交互界面*/
int SettingPage(void)
{
	uint8_t refresh = 1;	// 显示刷新

	while(1)
	{
		// 存储确认键被按下时setflag的值的临时变量，默认为无效值0
		uint8_t setflag_temp = 0;
		
		/* 按键处理 */
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))// 上键
		{
			setflag --;
			if (setflag < 1) setflag = 3;
			refresh = 1;
		}		
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))// 下键
		{
			setflag ++;
			if (setflag > 3) setflag = 1;
			refresh = 1;
		}		
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))// 确认键
		{
			// 清屏OLED，准备跳转
			OLED_Clear();
			OLED_Update();
			
			setflag_temp = setflag;
		}
		
		/* 功能跳转 */
		// 返回上一级菜单
		if (setflag_temp == 1){return 0;}
		// 进入日期时间设置界面
		else if (setflag_temp == 2){SetTime();refresh = 1;}
		// 进入屏幕亮度设置界面
		else if (setflag_temp == 3){SetBrightness();refresh = 1;}
		
		/* 显示更新*/
		if (refresh)
		{
			switch(setflag)
			{
				// 光标在第一行"回车"键
				case 1:
				{
					Show_SettingPage_UI();
					OLED_ReverseArea(  0,  0, 16, 16);
					OLED_Update();
					
					break;
				}
				// 光标在第二行"日期时间设置"选项
				case 2:
				{
					Show_SettingPage_UI();
					OLED_ReverseArea(  0, 16, 96, 16);
					OLED_Update();
					
					break;
				}
				// 光标在第三行"亮度设置"选项
				case 3:
				{
					Show_SettingPage_UI();
					OLED_ReverseArea(  0, 32, 64, 16);
					OLED_Update();
					
					break;
				}
			}
			refresh = 0;
		}
	}
}
/**********************************************************/
/*----------------------------------------[E] 设置界面 [E]*/
/**********************************************************/


/**********************************************************/
/*[S] 滑动菜单界面 [S]------------------------------------*/
/**********************************************************/

// 上次选择的选项
uint8_t pre_selection = 0;
// 目标选项
uint8_t targrt_selection;
// 上次选项的x坐标
int8_t x_pre = 48;
// 图标移动速度
uint8_t Speed = 4;
// 移动标志位;1开始，0停止
uint8_t move_flag;
// 菜单选项数目(包括返回键)
#define NUM_SELECTION		7

// 滑动菜单动画函数(图标横向移动)
void Menu_Animation(void)
{
	OLED_Clear();
	OLED_ShowImage( 42, 10, 44, 44, Frame);
	// 倒数第一个图标到第一个图标
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
	// 第一个图标到倒数第一个图标
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
		// 如果前一个图标左移到左边缘
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
		// 如果前一个图标右移到右边缘
		if (x_pre >= 96)
		{
			pre_selection --;
			move_flag = 0;
			x_pre = 48;
		}
	}
	
	// 前二的图标
	OLED_ShowImage(x_pre - 96, 16, 32, 32, Menu_Graph[(pre_selection - 2 + NUM_SELECTION) % NUM_SELECTION ]);
	// 前一的图标
	OLED_ShowImage(x_pre - 48, 16, 32, 32, Menu_Graph[(pre_selection - 1 + NUM_SELECTION) % NUM_SELECTION ]);
	// 当前的图标
	OLED_ShowImage(x_pre, 16, 32, 32, Menu_Graph[pre_selection]);
	// 后一的图标
	OLED_ShowImage(x_pre + 48, 16, 32, 32, Menu_Graph[(pre_selection + 1 + NUM_SELECTION) % NUM_SELECTION ]);
	// 后二的图标
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
	// 此函数外置，保证从功能中退出后能重新显示菜单界面
	Menu_Animation();
}

// 转场函数(图标向下移动)
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

/* 界面逻辑 */
int Menu(void)
{
	// move_flag=1;DirectFlag=0;使得进入菜单时处在退回键处
	move_flag = 1;
	// 图标移动标志位；1移动到上一项，2移动到下一项，默认为无效值0
	uint8_t DirectFlag = 0;
	
	while(1)
	{
		// 存储确认键被按下时menu_falg的值的临时变量，默认为无效值0
		uint8_t menu_falg_temp = 0;		
	
		/* 按键处理 */
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))// 上键
		{
			DirectFlag = 1;
			move_flag = 1;
			
			menu_falg --;
			if (menu_falg < 1) menu_falg = NUM_SELECTION;
		}		
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))// 下键
		{
			DirectFlag = 2;
			move_flag = 1;
			
			menu_falg ++;
			if (menu_falg > NUM_SELECTION ) menu_falg = 1;
		}	
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))// 确认键
		{
			// 清屏OLED，准备跳转
			OLED_Clear();
			OLED_Update();
			
			menu_falg_temp = menu_falg;
		}
		
		/* 功能跳转 */
		// 返回上一级菜单
		if (menu_falg_temp == 1){return 0;}
		// 跳转至秒表("StopWacth.c")
		else if (menu_falg_temp == 2){MenuToFunction();StopWatch();}
		else if (menu_falg_temp == 3){MenuToFunction();LED_Flashlight();}
		else if (menu_falg_temp == 4){MenuToFunction();MPU6050();}
		else if (menu_falg_temp == 5){MenuToFunction();Game();}
		else if (menu_falg_temp == 6){MenuToFunction();Emoji();}
		else if (menu_falg_temp == 7){MenuToFunction();Gradienter();}
		
		/* 显示更新 */
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
/**********************************************************/
/*------------------------------------[E] 滑动菜单界面 [E]*/
/**********************************************************/
