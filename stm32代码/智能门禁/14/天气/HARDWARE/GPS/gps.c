#include "gps.h"
#include <string.h>
#include <stdio.h>

volatile char gps_buffer[GPS_BUFFER_SIZE];
volatile uint8_t gps_index = 0;

// 初始化USART2（PA2 TX, PA3 RX，和你A2/A3一致）
void GPS_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // PA2 TX（如要发送AT指令用，平时可不用）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA3 RX（GPS模块TX接这里）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;    // 一般为9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

void GPS_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

// 简单GPGGA语句解析（仅供参考，可以提取更多字段）
void GPS_Parse(char* buf)
{
    // 查找GPGGA语句
    char* gga = strstr(buf, "$GPGGA");
    if (gga)
    {
        char time[11], lat[12], ns, lon[13], ew;
        int ret = sscanf(gga, "$GPGGA,%10[^,],%11[^,],%c,%12[^,],%c", time, lat, &ns, lon, &ew);
        if (ret == 5)
        {
            // 这里直接printf输出，你可以换成OLED等
            printf("UTC: %s, Lat: %s %c, Lon: %s %c\r\n", time, lat, ns, lon, ew);
        }
    }
}

// 串口2中断服务
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        char data = USART_ReceiveData(USART2);

        if (gps_index < GPS_BUFFER_SIZE - 1)
        {
            gps_buffer[gps_index++] = data;

            // 检测一条NMEA语句结束
            if (data == '\n')
            {
                gps_buffer[gps_index] = '\0';
                // 这里可以解析
                GPS_Parse((char*)gps_buffer);
                gps_index = 0;
            }
        }
        else
        {
            gps_index = 0;
        }
    }
}
