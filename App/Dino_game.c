/*******************************************************************************
[菜单-游戏]谷歌断网恐龙(Dino)
控制方式：上键跳跃
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

// 碰撞检测函数
int isColliding(struct Object_Position *a,struct Object_Position *b)
{
	if((a->maxX > b->minX)&&(a->minX < b->maxX)&&(a->maxY > b->minY)&&(a->minY < b->maxY))
	{
		return 1; 	// 发生碰撞
	}
	return 0;		// 未发生碰撞
}

// 参数配置
#define INITIAL_LIVES  		3			// 初始生命值
#define JUMP_HEIGHT  	28				// 跳跃高度

// 游戏状态
#define STATE_PLAYING   	0			// 游戏中
#define STATE_PAUSED_PLAY 	1			// 游戏暂停 - 选中 "继续"
#define STATE_PAUSED_EXIT	2			// 游戏暂停 - 选中 "退出"
#define STATE_GAME_OVER 	3			// 游戏结束
static uint8_t state;	

static int score = 0;					// 分数值
static int8_t lives;					// 生命值
static uint8_t cloud_pos = 0;
static uint16_t ground_pos = 0;
static uint8_t barrier_flag = 0;		// 障碍物存在类型
static uint8_t barrier_pos = 0;			// 障碍物位置
static struct Object_Position barrier;	// 障碍物碰撞箱
static uint8_t dino_jump_flag = 0; 		// 小恐龙跳跃状态 0奔跑 1跳跃
static struct Object_Position dino;		// 小恐龙碰撞箱
static uint8_t colliding_state;			// 碰撞进行状态 0未碰撞/碰撞完 1碰撞中
static uint16_t jump_t = 0;
static uint8_t jump_pos = 0;
double pi=3.1415927;

/* 显示更新（一帧） */
static void Render(void)
{
	OLED_Clear();
	
	/* 顶部 HUD：分数与生命 */
	OLED_ShowString(  0,  0, "SCORE", OLED_6X8);
	OLED_ShowNum(    30,  0, score, 5, OLED_6X8);
	OLED_ShowString( 90,  0, "LIFE", OLED_6X8);
	OLED_ShowNum(   114,  0, lives, 1, OLED_6X8);
	
	if (state == STATE_GAME_OVER)
	{
		/* 游戏结束 */
		OLED_ShowString( 28, 24, "Game Over", OLED_8X16);
		OLED_ShowString(  6, 45, "Hit confirm to exit", OLED_6X8);
	}
	else
	{
		/* 云朵 */
		OLED_ShowImage( 127-cloud_pos, 9, 16, 8, Cloud);
		
		/* 地面 */
		// 直接操控OLED显存数组OLED_DisplayBuf显示过长的地面图像
		// 将这片128个数据写入OLED_DisplayBuf数组
		if (ground_pos < 128)
		{
			for (uint8_t i=0;i < 128;i ++)
			{
				OLED_DisplayBuf[7][i] = Ground[i + ground_pos];
			}
		}
		else
		{
			for (uint8_t i=0;i < 255-ground_pos;i ++)
			{
				OLED_DisplayBuf[7][i] = Ground[i + ground_pos];
			}
			for (uint8_t i=255-ground_pos;i < 128;i ++)
			{
				OLED_DisplayBuf[7][i] = Ground[i - (255-ground_pos)];
			}
		}
		
		/* 障碍物 */
		OLED_ShowImage( 127-barrier_pos, 44, 16, 18, Barrier[barrier_flag]);
		
		/* 恐龙 */
		if(dino_jump_flag == 0)
		{
			if (cloud_pos % 2 == 0){OLED_ShowImage(  0, 44, 16, 18, Dino[0]);}// 借用Cloud_Pos
			else {OLED_ShowImage(  0, 44, 16, 18, Dino[1]);}
		}
		else
		{
			OLED_ShowImage(  0, 44-jump_pos, 16, 18, Dino[2]);
		}
		
		/* 暂停提示 */
		if (state == STATE_PAUSED_PLAY)
		{
			OLED_ShowString(  43, 24, "PAUSE", OLED_8X16);
			OLED_ShowString(  36, 45, "PLAY", OLED_6X8);
			OLED_ShowString(  66, 45, "EXIT", OLED_6X8);
			OLED_ReverseArea( 36, 44, 25,  9);
			
		}
		else if (state == STATE_PAUSED_EXIT)
		{
			OLED_ShowString(  43, 24, "PAUSE", OLED_8X16);
			OLED_ShowString(  36, 45, "PLAY", OLED_6X8);
			OLED_ShowString(  66, 45, "EXIT", OLED_6X8);
			OLED_ReverseArea( 66, 44, 25,  9);
		}
	}

	OLED_Update();
}

static uint8_t Score_Count, Ground_Count, Cloud_Count;
/* 物理更新 */
static void Physics(void)
{
	if (state != STATE_PLAYING) return;
	
	Score_Count ++;
	Ground_Count ++;
	Cloud_Count ++;
	
	/* 分数递增 */
	if (Score_Count >= 10) 		// 10ms * 10
	{
		Score_Count = 0;
		score ++;
	}
	
	/* 云移动 */
	if (Cloud_Count >= 5)		// 10ms * 5
	{
		Cloud_Count = 0;
		cloud_pos ++;
		if (cloud_pos >= 200){cloud_pos = 0;}
	}
	
	/* 地面和障碍物移动 */
	// 随机障碍物类型,取值为0、1、2
	if(barrier_pos >= 143){barrier_flag = rand()%3;}
	if (Ground_Count >= 2) 		// 10ms * 2
	{
		Ground_Count = 0;
		ground_pos ++;
		if (ground_pos >= 256){ground_pos = 0;}
		barrier_pos ++;
		if (barrier_pos >= 144){barrier_pos = 0;}
	}
	// 障碍物碰撞箱更新
	// 正常碰撞箱 16 * 18
//	barrier.minX = 127-barrier_pos;
//	barrier.maxX = 143-barrier_pos;
//	barrier.minY = 44;
//	barrier.maxY = 62;
	// 碰撞箱 14 * 16
	barrier.minX = 128-barrier_pos;
	barrier.maxX = 142-barrier_pos;
	barrier.minY = 45;
	barrier.maxY = 61;
	
	/* 恐龙跳 */
	if (Key_Check(KEY_NAME_UP,KEY_SINGLE))// 上键
	{
		dino_jump_flag = 1;
	}
	Key_Check(KEY_NAME_DOWN,KEY_SINGLE);// 下键
	if (dino_jump_flag == 1) 	// 10ms * 1
	{
		jump_t ++;
		if (jump_t >= 100) 
		{
			jump_t = 0;
			dino_jump_flag = 0;
		}
	}
	jump_pos = JUMP_HEIGHT * sin((float)pi*jump_t/100);
	
	// 恐龙碰撞箱更新
	// 正常碰撞箱 16 * 18
//	dino.minX = 0;
//	dino.maxX = 16;
//	dino.minY = 44-jump_pos;
//	dino.maxY = 62-jump_pos;
	// 碰撞箱 14 * 16 
	dino.minX = 1;
	dino.maxX = 15;
	dino.minY = 45-jump_pos;
	dino.maxY = 61-jump_pos;
	
	/* 碰撞检测 */
	if(colliding_state == 0 && isColliding( &dino, &barrier) == 1)
	{
		colliding_state = 1;
		lives --;
		if (lives <= 0)
		{
			state = STATE_GAME_OVER;
		}
	}
	else if (colliding_state == 1 && isColliding( &dino, &barrier) == 0)
	{
		colliding_state = 0;
	}
}

/* 初始化游戏 */
void DinoGame_Init(void)
{
	score = 0;
	lives = INITIAL_LIVES;
	ground_pos = barrier_pos = cloud_pos = jump_pos = 0;
	Score_Count = 0, Ground_Count = 0, Cloud_Count = 0;
	dino_jump_flag = 0;
	jump_t = 0;
	state = STATE_PLAYING;
	colliding_state = 0;
}

/* 游戏主循环 */
int DinoGame(void)
{
	// 时间参考值重置
	Time_Count1 = 0;
	Time_Count2 = 0;
	
	while(1)
	{
		if (state == STATE_PLAYING)
		{
			// 暂停 光标在 "继续"
			if (Key_Check(KEY_NAME_COMFIRM, KEY_SINGLE)) 
			{
				state = STATE_PAUSED_PLAY;
			}
		}
		else if (state == STATE_PAUSED_PLAY)
		{
			// 暂停 光标在 "退出"
			if (Key_Check(KEY_NAME_UP,KEY_SINGLE) ||
				Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
			{
				state = STATE_PAUSED_EXIT;
			}
			// 取消暂停
			else if (Key_Check(KEY_NAME_COMFIRM, KEY_SINGLE)) 
			{
				state = STATE_PLAYING;
			}
		}
		else if (state == STATE_PAUSED_EXIT)
		{
			// 暂停 光标在 "继续"
			if (Key_Check(KEY_NAME_UP,KEY_SINGLE) ||
				Key_Check(KEY_NAME_DOWN,KEY_SINGLE))
			{
				state = STATE_PAUSED_PLAY;
			}
			// 确认退出
			else if (Key_Check(KEY_NAME_COMFIRM, KEY_SINGLE)) 
			{
				OLED_Clear();
				OLED_Update();
				return 0;
			}
		}
		else if (state == STATE_GAME_OVER)
		{
			Key_Check(KEY_NAME_UP,KEY_SINGLE);// 上键
			Key_Check(KEY_NAME_DOWN,KEY_SINGLE);// 下键
			// 确认退出
			if (Key_Check(KEY_NAME_COMFIRM, KEY_SINGLE))
			{			
				OLED_Clear();
				OLED_Update();
				return 0;
			}
		}
		
		/* 显示更新 */
		if (Time_Count1 >= 40)	// 1ms * 40 显示更新周期
		{
			Time_Count1 -= 40;
			Render();
		}
		
		/* 物理更新 */ 
		if (Time_Count2 >= 10) // 1ms * 10 物理更新周期
		{
			Time_Count2 -= 10;
			Physics();
		}
	}
}
