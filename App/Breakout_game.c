/*******************************************************************************
[菜单-游戏]打砖块(Breakout)
控制方式：上/下键移动挡板，确认键发射球/暂停
*******************************************************************************/


#include "stm32f10x.h"
#include "common_headfile.h"

// 参数配置
#define BALL_SIZE       	3		// 球大小
#define PADDLE_WIDTH    	20		// 球拍宽度
#define PADDLE_HEIGHT   	2		// 球拍高度
#define PADDLE_Y        	60		// 球拍Y坐标
#define PADDLE_SPEED    	3		// 球拍移动速度

#define INITIAL_LIVES  		5		// 初始生命值

#define BRICK_WIDTH     	14		// 砖块长度
#define BRICK_HEIGHT    	4		// 砖块宽度
#define BRICK_COLS      	8		// 砖块列数
#define BRICK_ROWS      	4		// 砖块行数
#define BRICK_OFFSET_X  	1		// 砖块起始X偏移
#define BRICK_OFFSET_Y  	12		// 砖块起始Y偏移
#define BRICK_GAP_X     	2		// 砖块X方向间距
#define BRICK_GAP_Y     	1		// 砖块Y方向间距

// 游戏状态
#define STATE_WAIT      	0		// 等待发球
#define STATE_PLAYING   	1		// 游戏中
#define STATE_PAUSED_PLAY 	2		// 游戏暂停 - 选中 "继续"
#define STATE_PAUSED_EXIT	3		// 游戏暂停 - 选中 "退出"
#define STATE_GAME_OVER 	4		// 游戏结束（失败）
#define STATE_WIN       	5		// 游戏胜利
static uint8_t state;				// 游戏状态

static uint8_t bricks[BRICK_ROWS][BRICK_COLS];	// 砖块：1存在，0已消除
static int16_t score;				// 分数值
static int8_t lives;				// 生命值
static int16_t paddle_x;			// 球拍坐标
static float ball_x, ball_y;		// 球坐标
static float vx, vy;				// 球速


/* 重置球 */
// 球贴回挡板并停止
static void ResetBall(void)
{
	ball_x = paddle_x + PADDLE_WIDTH / 2 - BALL_SIZE / 2;
	ball_y = PADDLE_Y - BALL_SIZE;
	vx = 0.0f;
	vy = 0.0f;
}

/* 发射球 */
static void LaunchBall(void)
{
	vx = 0.75f;
	vy = -1.1f;
}

/* 显示更新（一帧） */
static void Render(void)
{
	OLED_Clear();
	
	/* 顶部 HUD：分数与生命 */
	OLED_ShowString(  0,  0, "SCORE", OLED_6X8);
	OLED_ShowNum(    30,  0, score, 3, OLED_6X8);
	OLED_ShowString( 90,  0, "LIFE", OLED_6X8);
	OLED_ShowNum(   114,  0, lives, 1, OLED_6X8);
	
	if (state == STATE_GAME_OVER)
	{
		/* 游戏结束 */
		OLED_ShowString( 28, 24, "Game Over", OLED_8X16);
		OLED_ShowString(  6, 45, "Hit confirm to exit", OLED_6X8);
	}
	else if (state == STATE_WIN)
	{
		/* 游戏胜利 */
		OLED_ShowString( 40, 24, "You Win", OLED_8X16);
		OLED_ShowString(  6, 45, "Hit confirm to exit", OLED_6X8);
	}
	else
	{
		/* 砖块 */
		for (uint8_t r = 0; r < BRICK_ROWS; r ++)
		{
			for (uint8_t c = 0; c < BRICK_COLS; c ++)
			{
				if (bricks[r][c])
				{
					OLED_DrawRectangle(BRICK_OFFSET_X + c * (BRICK_WIDTH + BRICK_GAP_X),
					                   BRICK_OFFSET_Y + r * (BRICK_HEIGHT + BRICK_GAP_Y),
					                   BRICK_WIDTH, BRICK_HEIGHT, OLED_FILLED);
				}
			}
		}
		
		/* 挡板 */
		OLED_DrawRectangle(paddle_x, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT, OLED_FILLED);
		
		/* 球 */
		OLED_DrawRectangle((int16_t)ball_x, (int16_t)ball_y, BALL_SIZE, BALL_SIZE, OLED_FILLED);
		
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

/* 物理更新 */
static void Physics(void)
{
	/* 挡板移动（等待发射/游戏中）*/
	if (state == STATE_WAIT || state == STATE_PLAYING)
	{
		if (Key_Check(KEY_NAME_UP, KEY_HOLD))   { paddle_x -= PADDLE_SPEED; }
		if (Key_Check(KEY_NAME_DOWN, KEY_HOLD)) { paddle_x += PADDLE_SPEED; }
		if (paddle_x < 0) paddle_x = 0;
		if (paddle_x > 128 - PADDLE_WIDTH) paddle_x = 128 - PADDLE_WIDTH;
	}
	
	/* 等待发射：球跟随挡板 */
	if (state == STATE_WAIT)
	{
		ball_x = paddle_x + PADDLE_WIDTH / 2 - BALL_SIZE / 2;
		ball_y = PADDLE_Y - BALL_SIZE;
		return;
	}
	
	if (state != STATE_PLAYING) return;
	
	/* 球移动 */
	ball_x += vx;
	ball_y += vy;
	
	/* 左右墙反弹 */
	if (ball_x < 0) { ball_x = 0; vx = -vx; }
	if (ball_x + BALL_SIZE > 128) { ball_x = 128 - BALL_SIZE; vx = -vx; }
	
	/* 顶墙反弹 */
	if (ball_y < 0) { ball_y = 0; vy = -vy; }
	
	/* 挡板反弹（球向下，从上方撞挡板）*/
	if (vy > 0 && ball_y + BALL_SIZE >= PADDLE_Y && ball_y <= PADDLE_Y
		&& ball_x + BALL_SIZE > paddle_x && ball_x < paddle_x + PADDLE_WIDTH)
	{
		/* 根据落点偏移调整水平速度，边缘斜向弹开 */
		float hit = (ball_x + BALL_SIZE / 2.0f - (paddle_x + PADDLE_WIDTH / 2.0f)) / (PADDLE_WIDTH / 2.0f);
		if (hit > 1.0f) hit = 1.0f;
		if (hit < -1.0f) hit = -1.0f;
		vx = hit * 1.5f;
		if (vx > -0.3f && vx < 0.3f) vx = (hit >= 0 ? 0.3f : -0.3f);
		vy = -vy;
		ball_y = PADDLE_Y - BALL_SIZE;
	}
	
	/* 砖块碰撞 */
	uint8_t brick_hit = 0;
	for (uint8_t r = 0; r < BRICK_ROWS && !brick_hit; r ++)
	{
		for (uint8_t c = 0; c < BRICK_COLS; c ++)
		{
			if (!bricks[r][c]) continue;
			
			int16_t bx = BRICK_OFFSET_X + c * (BRICK_WIDTH + BRICK_GAP_X);
			int16_t by = BRICK_OFFSET_Y + r * (BRICK_HEIGHT + BRICK_GAP_Y);
			
			if (ball_x + BALL_SIZE > bx && ball_x < bx + BRICK_WIDTH
				&& ball_y + BALL_SIZE > by && ball_y < by + BRICK_HEIGHT)
			{
				bricks[r][c] = 0;
				score += 10;
				
				/* 用球心相对砖心的偏移判断反转轴 */
				float cx = ball_x + BALL_SIZE / 2.0f;
				float cy = ball_y + BALL_SIZE / 2.0f;
				float bcx = bx + BRICK_WIDTH / 2.0f;
				float bcy = by + BRICK_HEIGHT / 2.0f;
				float dx = cx - bcx; if (dx < 0) dx = -dx;
				float dy = cy - bcy; if (dy < 0) dy = -dy;
				if (dy > dx) vy = -vy;
				else         vx = -vx;
				
				brick_hit = 1;
				break;
			}
		}
	}
	
	/* 漏球扣命 */
	if (ball_y >= 63)
	{
		lives --;
		if (lives <= 0)
		{
			state = STATE_GAME_OVER;
		}
		else
		{
			state = STATE_WAIT;
			ResetBall();
		}
	}
	
	/* 过关判定 */
	if (state == STATE_PLAYING)
	{
		uint8_t remain = 0;
		for (uint8_t r = 0; r < BRICK_ROWS; r ++)
		{
			for (uint8_t c = 0; c < BRICK_COLS; c ++)
			{
				if (bricks[r][c]) remain ++;
			}
		}
		if (remain == 0) state = STATE_WIN;
	}
}

/* 初始化游戏 */
void BreakoutGame_Init(void)
{
	score = 0;
	lives = INITIAL_LIVES;
	paddle_x = (128 - PADDLE_WIDTH) / 2;
	state = STATE_WAIT;
	
	for (uint8_t r = 0; r < BRICK_ROWS; r ++)
	{
		for (uint8_t c = 0; c < BRICK_COLS; c ++)
		{
			bricks[r][c] = 1;
		}
	}
	
	ResetBall();
}

/* 游戏主循环 */
int BreakoutGame(void)
{
	// 时间参考值重置
	Time_Count1 = 0;
	Time_Count2 = 0;
	
	while (1)
	{
		if (state == STATE_WAIT)
		{
			Key_Check(KEY_NAME_UP,KEY_SINGLE);// 上键
			Key_Check(KEY_NAME_DOWN,KEY_SINGLE);// 下键
			// 发射
			if (Key_Check(KEY_NAME_COMFIRM, KEY_SINGLE)) 
			{
				LaunchBall(); 
				state = STATE_PLAYING; 
			}
		}
		else if (state == STATE_PLAYING)
		{
			Key_Check(KEY_NAME_UP,KEY_SINGLE);// 上键
			Key_Check(KEY_NAME_DOWN,KEY_SINGLE);// 下键
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
		else if (state == STATE_GAME_OVER || state == STATE_WIN)
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
		
		/* 物理更新 */
		if (Time_Count2 >= 20) 	// 1ms * 20 物理更新周期
		{
			Time_Count2 -= 20;
			Physics();
		}
		
		/* 显示更新 */
		if (Time_Count1 >= 40)	// 1ms * 40 显示更新周期
		{
			Time_Count1 -= 40;
			Render();
		}
	}
}
