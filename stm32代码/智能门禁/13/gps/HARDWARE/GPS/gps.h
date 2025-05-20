#ifndef __GPS_H
#define __GPS_H

#include "stm32f10x.h"

#define GPS_BUFFER_SIZE 128

void GPS_Init(uint32_t baudrate);
void GPS_NVIC_Config(void);
void GPS_Parse(char* buf);    // 你可以根据自己需求自行改进
extern volatile char gps_buffer[GPS_BUFFER_SIZE];

#endif
