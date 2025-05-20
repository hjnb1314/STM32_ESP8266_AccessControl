#ifndef __WEATHER_H
#define __WEATHER_H

#include "stm32f10x.h"

void Weather_Init(void);     // 串口2初始化
void Weather_Task(void);     // 解析与显示任务

extern char weather_type[16];
extern char weather_temp[16];

#endif
