#ifndef __RELAY_H
#define __RELAY_H

#include "stm32f10x.h"

// 初始化连接在PB12的继电器
void Relay_Init(void);

// 打开连接在PB12的继电器
void Relay_On(void);

// 关闭连接在PB12的继电器
void Relay_Off(void);

#endif    