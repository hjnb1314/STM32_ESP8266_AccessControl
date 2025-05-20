#ifndef __PWM_H
#define __PWM_H

void PWM_Init(void);                    // 初始化TIM2通道3，PA2
void PWM_SetCompare3(uint16_t Compare); // 设置PWM占空比（舵机角度脉宽）

#endif
