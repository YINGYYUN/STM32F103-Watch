/*******************************************************************************
[菜单]游戏界面
显示游戏选项
*******************************************************************************/


#include "stm32f10x.h"
#include "common_headfile.h"

void Show_Game_UI(void)
{
	OLED_ShowImage(  0,  0, 16, 16, Return);
	OLED_ShowString(  0, 16, "谷歌小恐龙", OLED_8X16);
	OLED_ShowString(  0, 32, "打砖块", OLED_8X16);
}

int Game(void)
{	
	// game界面选项标志位
	uint8_t game_flag = 1;
	
	uint8_t refresh = 1;	// UI刷新标志位
	
	while(1)
	{
		// 存储确认键被按下时game_flag的值的临时变量，默认为无效值0
		uint8_t game_flag_temp = 0;
		
		/* 按键处理 */
		if (Key_Check(KEY_NAME_UP,KEY_SINGLE))// 上键
		{
			game_flag --;
			if (game_flag < 1)game_flag = 3;
			refresh = 1;
		}
		else if (Key_Check(KEY_NAME_DOWN,KEY_SINGLE))// 下键
		{
			game_flag ++;
			if (game_flag > 3)game_flag = 1;
			refresh = 1;
		}
		else if(Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))// 确认键
		{
			game_flag_temp = game_flag;
		}
	
		/* 功能跳转 */
		// 返回上一级菜单
		if (game_flag_temp == 1){OLED_Clear();return 0;}
		else if (game_flag_temp == 2){DinoGame_Init();DinoGame();refresh=1;}
		else if (game_flag_temp == 3){BreakoutGame_Init();BreakoutGame();refresh=1;}
		
		/* 显示更新 */
		if (refresh)
		{
			switch(game_flag)
			{
				case 1:
				{
					OLED_Clear();
					Show_Game_UI();
					OLED_ReverseArea(  0, 0, 16, 16);
					OLED_Update();
					
					break;
				}
				case 2:
				{
					OLED_Clear();
					Show_Game_UI();
					OLED_ReverseArea(  0, 16, 80, 16);
					OLED_Update();
					
					break;
				}
				case 3:
				{
					OLED_Clear();
					Show_Game_UI();
					OLED_ReverseArea(  0, 32, 48, 16);
					OLED_Update();
					
					break;
				}
			}
			refresh = 0;
		}
	}
}
