#include "stm32f10x.h"                  // Device header

#include "Delay.h"
#include "Timer.h"

#include "OLED.h"
#include "menu.h"

int main(void)
{
	/*OLED初始化*/
	OLED_Init();
	Peripheral_Init();
	
	Timer_Init();
	
	OLED_Clear();
	
	uint8_t clkflag1;
	
	while (1)
	{
		clkflag1 = First_Page_Clock();
		//跳转到菜单
		if (clkflag1 == 1){Menu();}
		//跳转到设置
		else if (clkflag1 == 2){SettingPage();}
	}
}

/*TIM2定时器定时中断*/
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Key_Tick();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
