#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f10x.h"

void BEEP_Init(void);
void BEEP_On(void);
void BEEP_Off(void);
void BEEP_Toggle(void);

#endif /* __BEEP_H */
