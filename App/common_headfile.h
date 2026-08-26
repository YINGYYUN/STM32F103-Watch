/*******************************************************************************
公共头文件
*******************************************************************************/


#ifndef __COMMON_HEADFILE_H__
#define __COMMON_HEADFILE_H__

#include "stm32f10x.h"

/* 应用层 */
#include "menu.h"
#include "SetTime_page.h"
#include "SetBrightness_page.h"
#include "StopWatch_page.h"
#include "LEDFlashlight_page.h"
#include "MPU6050_page.h"
#include "Game_page.h"
#include "Emoji_page.h"
#include "Gradienter_page.h"

/* 游戏 */
#include "Dino_game.h"
#include "Breakout_game.h"

/* 驱动层 */
#include "OLED.h"
#include "LED.h"
#include "Key.h"
#include "MPU6050.h"
#include "AD.h"

/* 算法库 */
#include "Roll_Clock.h"
#include "mpu6050_Analysis.h"

/* 时间参考值 */
// 1ms ++
extern uint16_t Time_Count1;
extern uint16_t Time_Count2;

#endif
