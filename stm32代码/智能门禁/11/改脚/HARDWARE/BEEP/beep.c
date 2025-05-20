#include "beep.h"

/**
  * @brief  初始化 PB4 为推挽输出，低电平触发
  */
void BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIOB 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 配置 PB4 为推挽输出，速率 50MHz */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 默认关闭蜂鸣器（高电平） */
    GPIO_SetBits(GPIOB, GPIO_Pin_4);
}

/**
  * @brief  打开蜂鸣器（低电平，响）
  */
void BEEP_On(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_4);
}

/**
  * @brief  关闭蜂鸣器（高电平，不响）
  */
void BEEP_Off(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_4);
}

/**
  * @brief  翻转蜂鸣器状态
  */
void BEEP_Toggle(void)
{
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_4) == 0)
        GPIO_SetBits(GPIOB, GPIO_Pin_4);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_4);
}
