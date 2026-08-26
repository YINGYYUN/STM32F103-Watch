/*******************************************************************************
[菜单-游戏]谷歌断网恐龙
*******************************************************************************/


#include "stm32f10x.h"
#include "common_headfile.h"
#include "Delay.h"
#include <stdlib.h>
#include <math.h>

// 碰撞箱
struct Object_Position {
	uint8_t minX, minY, maxX, maxY;
};

// 分数显示
int Score = 0;
void Show_Score(void)
{
	OLED_ShowString(  0,  0, "SCORE", OLED_6X8);
	OLED_ShowNum(    30,  0, Score, 5, OLED_6X8);
}

// 地面显示
uint16_t Ground_Pos = 0;
// 直接操控OLED显存数组OLED_DisplayBuf显示过长的地面图像
void Show_Ground(void)
{
	// 直接将这片128个数据写入OLED_DisplayBuf数组
	if (Ground_Pos < 128)
	{
		for (uint8_t i=0;i < 128;i ++)
		{
			OLED_DisplayBuf[7][i] = Ground[i + Ground_Pos];
		}
	}
	else
	{
		for (uint8_t i=0;i < 255-Ground_Pos;i ++)
		{
			OLED_DisplayBuf[7][i] = Ground[i + Ground_Pos];
		}
		for (uint8_t i=255-Ground_Pos;i < 128;i ++)
		{
			OLED_DisplayBuf[7][i] = Ground[i - (255-Ground_Pos)];
		}
	}
}

// 障碍物显示
uint8_t Barrier_flag = 0;	
uint8_t Barrier_Pos = 0;
// 障碍物碰撞箱
struct Object_Position barrier;

void Show_Barrier(void)
{
	// 随机障碍物类型
	// 取值为0、1、2
	if(Barrier_Pos >= 143){Barrier_flag = rand()%3;}
	
	// 显示更新
	OLED_ShowImage( 127-Barrier_Pos, 44, 16, 18, Barrier[Barrier_flag]);
	
	// 碰撞箱更新
	// 正常碰撞箱 16 * 18
//	barrier.minX = 127-Barrier_Pos;
//	barrier.maxX = 143-Barrier_Pos;
//	barrier.minY = 44;
//	barrier.maxY = 62;
	// 碰撞箱 14 * 16
	barrier.minX = 128-Barrier_Pos;
	barrier.maxX = 142-Barrier_Pos;
	barrier.minY = 45;
	barrier.maxY = 61;
}

// 云朵显示
uint8_t Cloud_Pos = 0;
void Show_Cloud(void)
{
	OLED_ShowImage( 127-Cloud_Pos, 9, 16, 8, Cloud);
}

// 恐龙显示
uint8_t dino_jump_flag = 0;
// 0 奔跑
// 1 跳跃
uint16_t jump_t = 0;
uint8_t Jump_Pos = 0;
double pi=3.1415927;
// 小恐龙碰撞箱
struct Object_Position dino;

void Show_Dino(void)
{
	/* 按键处理 */
	if (Key_Check(KEY_NAME_UP,KEY_SINGLE))// 上键
	{
		dino_jump_flag = 1;
	}
	Key_Check(KEY_NAME_DOWN,KEY_SINGLE);// 下键
	Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE);// 确认键
	
	Jump_Pos = 28 * sin((float)pi*jump_t/100);
	
	// 显示更新
	if(dino_jump_flag == 0)
	{
		// 借用Cloud_Pos
		if (Cloud_Pos % 2 == 0){OLED_ShowImage(  0, 44, 16, 18, Dino[0]);}
		else {OLED_ShowImage(  0, 44, 16, 18, Dino[1]);}
	}
	else
	{
		OLED_ShowImage(  0, 44-Jump_Pos, 16, 18, Dino[2]);
	}
	
	// 更新碰撞箱
	// 正常碰撞箱 16 * 18
//	dino.minX = 0;
//	dino.maxX = 16;
//	dino.minY = 44-Jump_Pos;
//	dino.maxY = 62-Jump_Pos;
	// 碰撞箱 14 * 16 
	dino.minX = 1;
	dino.maxX = 15;
	dino.minY = 45-Jump_Pos;
	dino.maxY = 61-Jump_Pos;
}

// 碰撞检测函数，传入两个结构体的地址
int isColliding(struct Object_Position *a,struct Object_Position *b)
{
	if((a->maxX > b->minX)&&(a->minX < b->maxX)&&(a->maxY > b->minY)&&(a->minY < b->maxY))
	{
		// 发生碰撞
		return 1;
	}
	// 未发生碰撞
	return 0;
}

// 显示总调用
int DinoGame(void)
{
	int game_stop_flag = 0;
	// 时间参考值重置
	Time_Count1 = 0;
	Time_Count2 = 0;

	static uint8_t Score_Count = 0, Ground_Count = 0, Cloud_Count = 0;
	
	while(1)
	{
		/* 显示更新 */
		if (game_stop_flag == 0 && Time_Count1 >= 40) // 1ms * 40 显示更新周期
		{
			Time_Count1 = 0;
			
			// 渲染耗时可能在 15~20ms，显示刷新调用时间间隔需要大于这个值
			OLED_Clear();
			Show_Score();
			Show_Ground();
			Show_Barrier();
			Show_Cloud();
			Show_Dino();
			OLED_Update();
			
			if(isColliding( &dino, &barrier) == 1)
			{
				game_stop_flag = 1;
			}
		}
		if (game_stop_flag == 1)
		{	
			OLED_Clear();
			Show_Score();
			OLED_ShowString( 28, 24, "Game Over", OLED_8X16);
			OLED_ShowString(  6, 45, "Hit confirm to exit", OLED_6X8);
			OLED_Update();
			
			game_stop_flag = 2;
		}
		if (game_stop_flag == 2)
		{
			Key_Check(KEY_NAME_UP,KEY_SINGLE);
			Key_Check(KEY_NAME_DOWN,KEY_SINGLE);
			// 按确认键退出
			if (Key_Check(KEY_NAME_COMFIRM,KEY_SINGLE))
			{
				OLED_Clear();
				OLED_Update();
				
				return 0;
			}
		}
		
		/* 物理更新 */ 
		if (game_stop_flag == 0 && Time_Count2 >= 10) // 1ms * 10 物理更新周期
		{
			Time_Count2 -= 10;
			
			Score_Count ++;
			Ground_Count ++;
			Cloud_Count ++;
			
			if (Score_Count >= 10) 		// 10ms * 10
			{
				Score_Count = 0;
				Score ++;
			}
			
			if (Ground_Count >= 2) 		// 10ms * 2
			{
				Ground_Count = 0;
				Ground_Pos ++;
				if (Ground_Pos >= 256){Ground_Pos = 0;}
				Barrier_Pos ++;
				if (Barrier_Pos >= 144){Barrier_Pos = 0;}
			}
			
			if (Cloud_Count >= 5)		// 10ms * 5
			{
				Cloud_Count = 0;
				Cloud_Pos ++;
				if (Cloud_Pos >= 200){Cloud_Pos = 0;}
			}
			if (dino_jump_flag == 1) 	// 10ms * 1
			{
				jump_t ++;
				if (jump_t >= 100) 
				{
					jump_t = 0;
					dino_jump_flag = 0;
				}
			}
		}
	}
}

/* 初始化游戏 */
void DinoGame_Init(void)
{
	// 分数清零
	Score = 0;
	// 位置重置
	Ground_Pos = Barrier_Pos = Cloud_Pos = Jump_Pos = 0;
	// 跳跃状态重置
	dino_jump_flag = 0;
	jump_t = 0;
}
