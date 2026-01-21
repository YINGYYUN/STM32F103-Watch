#include "stm32f10x.h"                  // Device header

#include "MyRTC.h"

#include "OLED.h"
#include "Key.h"


uint8_t Brightness_Level = 4;
/*--------------------[S] 界面设计 [S]--------------------*/

void Show_SetBrightness_UI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_Printf(0, 16, OLED_8X16, "亮度:");
	OLED_ShowImage(48, 16, 16, 16, Brightness_Image);
	OLED_ShowImage(64, 16, 16, 16, Brightness_Image);
	OLED_ShowImage(80, 16, 16, 16, Brightness_Image);
	OLED_ShowImage(96, 16, 16, 16, Brightness_Image);
	OLED_ShowImage(112, 16, 16, 16, Brightness_Image);
	OLED_ReverseArea(48, 16, 16 * Brightness_Level, 16);
}
/*--------------------[E] 界面设计 [E]--------------------*/


/*--------------------[S] 亮度配置 [S]--------------------*/

//亮度档位映射表
static const uint8_t OLED_Brightness_contrastTable[6] = {
    0x00,    // 0 备用/极暗，不在正常循环内
    0x32,   // 1/5  ≈20%
    0x64,  // 2/5  ≈40%
    0x96,  // 3/5  ≈60%
    0xC8,  // 4/5  ≈80%
    0xFA   // 5/5  ≈100%
};

void Write_OLED_Brightness_CMD(uint8_t Brightness)
{
	//越界判定
	if (Brightness < 1) Brightness = 1;
    if (Brightness > 5) Brightness = 5;
	
	OLED_WriteCommand(0x81);                 // 对比度寄存器
    OLED_WriteCommand(OLED_Brightness_contrastTable[Brightness]);
}

int Set_OLED_Brightness(void)
{
	while(1)
	{
		//上键，亮度减
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			Brightness_Level --;
			if (Brightness_Level < 1)Brightness_Level = 1;
		}
		//下键，亮度加
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			Brightness_Level ++;
			if (Brightness_Level > 5)Brightness_Level = 5;
		}
		//确认键，退出
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			return 0;
		}
		
		//写入对比度（亮度）
		Write_OLED_Brightness_CMD(Brightness_Level);
		Show_SetBrightness_UI();
		OLED_Update();
	}
}
/*--------------------[E] 亮度配置 [E]--------------------*/


/*--------------------[S] 模块交互代码 [S]--------------------*/

//日期时间设置界面选项标志位
uint8_t set_brightness_flag = 1;

uint8_t SetBrightness(void)
{
	while(1)
	{
		//存储确认键被按下时set_brightness_flag的值的临时变量，默认为无效值0
		uint8_t set_brightness_flag_temp = 0;
		
		//上键
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))
		{
			set_brightness_flag --;
			if (set_brightness_flag <= 0) set_brightness_flag = 2;
		}
		//下键
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
		{
			set_brightness_flag ++;
			if (set_brightness_flag >= 3) set_brightness_flag = 1;
		}
		//确认键
		else if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
		{
			//清屏OLED，准备跳转
			OLED_Clear();
			OLED_Update();
			
			set_brightness_flag_temp = set_brightness_flag;
		}
		
		//返回上一级菜单
		if (set_brightness_flag_temp == 1){return 0;}
		//进入亮度设置
		else if (set_brightness_flag_temp == 2){Set_OLED_Brightness();}
			
		switch(set_brightness_flag)
		{
			//光标在第一行"回车"键
			case 1:
			{
				OLED_Clear();
				Show_SetBrightness_UI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				
				break;
			}
			//光标在第二行"亮度"选项
			case 2:
			{
				OLED_Clear();
				Show_SetBrightness_UI();
				OLED_ReverseArea(0, 16, 32, 16);
				OLED_Update();
				
				break;
			}
		}
	}
}
/*--------------------[E] 模块交互代码 [E]--------------------*/
