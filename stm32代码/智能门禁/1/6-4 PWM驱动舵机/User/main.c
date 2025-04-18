#include "stm32f10x.h"
#include "led.h"
#include "delay.h"
#include "OLED.h"
#include "button4_4.h"
#include "relay.h"
 
/*****************辰哥单片机设计******************
											STM32
 * 项目			:	4×4矩阵键盘实验                     
 * 版本			: V1.0
 * 日期			: 2024.9.5
 * MCU			:	STM32F103C8T6
 * 接口			:	参看button4_4.h							
 * BILIBILI	:	辰哥单片机设计
 * CSDN			:	辰哥单片机设计
 * 作者			:	辰哥 
**********************BEGIN***********************/
 
char value;
 
int main(void)
{ 
	Button4_4_Init();
	OLED_Init();
	Relay_Init();
	Delay_us(1000);
	
  while (1)
  {
		
		value = Button4_4_Scan();
		if(value!='\0')
		{
			OLED_ShowChar(1, 1, value);
			Delay_us(500);
	  if(value=='A')
			{
			Relay_On();
			}
			else if(value=='B'){
			Relay_Off();
			}
		}
		
		
  }
}