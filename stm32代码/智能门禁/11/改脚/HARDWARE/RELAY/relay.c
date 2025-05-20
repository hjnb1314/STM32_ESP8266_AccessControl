#include "relay.h"

void Relay_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 释放JTAG，允许PB3作为普通GPIO使用
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_3);  // 默认断开（高电平）
}

void Relay_On(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_3);  // 低电平吸合（灯亮）
}

void Relay_Off(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_3);   // 高电平断开
}

void Relay_Toggle(void)
{
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_3) == 0)
        GPIO_SetBits(GPIOB, GPIO_Pin_3);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_3);
}
