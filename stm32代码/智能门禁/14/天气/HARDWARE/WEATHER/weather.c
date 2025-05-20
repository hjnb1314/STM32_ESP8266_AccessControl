#include "weather.h"
#include "oled_iic.h"
#include <string.h>
#include <stdio.h>

char weather_type[16] = {0};
char weather_temp[16] = {0};
static char rx_buf[32] = {0};
static uint8_t rx_idx = 0;

// 辅助：去除字符串首部所有空格
/*static void strip_leading_space(char *str)
{
    char *p = str;
    while (*p == ' ') p++;
    if (p != str)
        memmove(str, p, strlen(p) + 1);
}*/

void Weather_Init(void)
{
    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // PA2=TX, PA3=RX
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    USART_InitTypeDef usart;
    usart.USART_BaudRate = 115200;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &usart);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = USART2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    USART_Cmd(USART2, ENABLE);
}

// 串口2中断服务函数
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
        char ch = USART_ReceiveData(USART2);
        if (ch == '\n') {    // 一包数据结束
            rx_buf[rx_idx] = '\0';
            // 解析数据（天气类型,温度）
            char *comma = strchr(rx_buf, ',');
            if (comma) {
                size_t type_len = comma - rx_buf;
                if (type_len < sizeof(weather_type)) {
                    strncpy(weather_type, rx_buf, type_len);
                    weather_type[type_len] = '\0';
                }
                strncpy(weather_temp, comma + 1, sizeof(weather_temp) - 1);
                weather_temp[sizeof(weather_temp)-1] = '\0';

                // 只保留温度数字
                char digits_only[16] = {0};
                int j = 0;
                for (int i = 0; weather_temp[i] != '\0'; i++) {
                    if (weather_temp[i] >= '0' && weather_temp[i] <= '9') {
                        digits_only[j++] = weather_temp[i];
                    }
                    if (j >= sizeof(digits_only) - 1) break;
                }
                strcpy(weather_temp, digits_only);
            }
            rx_idx = 0;
            memset(rx_buf, 0, sizeof(rx_buf));
        }
        else if (ch != '\r' && rx_idx < sizeof(rx_buf) - 1) {
            rx_buf[rx_idx++] = ch;
        }
    }
}


void Weather_Task(void)
{
    //OLED_Clear();
    //OLED_ShowCH(16, 0, (u8 *)"城市: 佛山");         // 第一行
    //OLED_ShowCH(16, 2, (u8 *)"天气:");
    OLED_ShowCH(45, 2, (u8 *)weather_type);          // 天气类型

    //OLED_ShowCH(16, 4, (u8 *)"温度:");
    OLED_ShowCH(16, 4, (u8 *)weather_temp);          // 温度

    //OLED_ShowCH(16, 6, (u8 *)"K15 返回");             // 菜单返回提示
}
