/*******************************************************************************
[首页]滚动时钟显示
参考项目：【【开源】oled 滚动时钟 3.0 版本，全新字体，代码精简，移植方便】 
* https://www.bilibili.com/video/BV1PpcEz5EUo/?share_source=copy_web&vd_source=befff269c2e0ef1583e2528659770521
*******************************************************************************/


#ifndef __ROLL_CLOCK_H__
#define __ROLL_CLOCK_H__

#include <stdint.h>

// 滚动时钟显示结构体
typedef struct {
	
	uint8_t Last;			// 旧值
	uint8_t New;			// 新值
					
	uint8_t Ge_Last;		// 个位旧值
	uint8_t Ge_New;			// 个位新值
	
	uint8_t Shi_Last;		// 十位旧值
	uint8_t Shi_New;		// 十位新值
	
	uint8_t Ge_Roll_Flag;	// 个位滚动标志位
	uint8_t Shi_Roll_Flag;	// 十位滚动标志位
	
	int8_t Ge_Y;			// 个位Y坐标
	int8_t Shi_Y;			// 十位Y坐标
	
}Clock;						

void 	Roll_Clock_Init	(void);
void 	Roll_Clock		(void);

#endif
