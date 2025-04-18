#ifndef __BUTTON4_4_H
#define	__BUTTON4_4_H
#include "stm32f10x.h"
#include "delay.h"
#include "math.h"
 
/*****************���絥Ƭ�����******************
											STM32
 * �ļ�			:	4��4�������h�ļ�                   
 * �汾			: V1.0
 * ����			: 2024.9.5
 * MCU			:	STM32F103C8T6
 * �ӿ�			:	������							
 * BILIBILI	:	���絥Ƭ�����
 * CSDN			:	���絥Ƭ�����
 * ����			:	����
**********************BEGIN***********************/
 
/***************�����Լ��������****************/
// 4��4������� GPIO�궨��
 
#define		BUTTON_GPIO_CLK											RCC_APB2Periph_GPIOA
#define 	BUTTON_ROW1_GPIO_PORT								GPIOA
#define 	BUTTON_ROW1_GPIO_PIN							  GPIO_Pin_0	
#define 	BUTTON_ROW2_GPIO_PORT								GPIOA
#define 	BUTTON_ROW2_GPIO_PIN								GPIO_Pin_1
#define 	BUTTON_ROW3_GPIO_PORT								GPIOA
#define 	BUTTON_ROW3_GPIO_PIN								GPIO_Pin_2
#define 	BUTTON_ROW4_GPIO_PORT								GPIOA
#define 	BUTTON_ROW4_GPIO_PIN								GPIO_Pin_3
 
#define 	BUTTON_COL1_GPIO_PORT								GPIOA
#define 	BUTTON_COL1_GPIO_PIN								GPIO_Pin_4
#define 	BUTTON_COL2_GPIO_PORT								GPIOA
#define 	BUTTON_COL2_GPIO_PIN								GPIO_Pin_5
#define 	BUTTON_COL3_GPIO_PORT								GPIOA
#define 	BUTTON_COL3_GPIO_PIN								GPIO_Pin_6
#define 	BUTTON_COL4_GPIO_PORT								GPIOA
#define 	BUTTON_COL4_GPIO_PIN								GPIO_Pin_7
 
	
/*********************END**********************/
 
void Button4_4_Init(void);
char Button4_4_Scan(void);
 
 
#endif