#ifndef __RELAY_H
#define __RELAY_H

#include "stm32f10x.h"

// ��ʼ��������PB12�ļ̵���
void Relay_Init(void);

// ��������PB12�ļ̵���
void Relay_On(void);

// �ر�������PB12�ļ̵���
void Relay_Off(void);

#endif    