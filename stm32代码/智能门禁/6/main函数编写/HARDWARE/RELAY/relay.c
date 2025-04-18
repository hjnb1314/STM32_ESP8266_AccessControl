#include "relay.h"

/**
  * @brief  配置 PB12 引脚为推挽输出，用于驱动继电器。
  * @param  None
  * @retval None
  */
void Relay_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 使能 GPIOB 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    /* 配置 PB12 为推挽输出，速率 50MHz */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 默认先让继电器断开（输出低电平） */
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

/**
  * @brief  继电器吸合（PB12 输出高电平）。
  * @param  None
  * @retval None
  */
void Relay_On(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

/**
  * @brief  继电器断开（PB12 输出低电平）。
  * @param  None
  * @retval None
  */
void Relay_Off(void)
{

	GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

/**
  * @brief  切换继电器的状态（如果当前为吸合，则断开；如果当前为断开，则吸合）。
  * @param  None
  * @retval None
  */
void Relay_Toggle(void)
{
    /* 读取当前 PB12 的输出状态 */
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_12))
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);  // 如果是高电平，就置低
    }
    else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_12);    // 如果是低电平，就置高
    }
}
