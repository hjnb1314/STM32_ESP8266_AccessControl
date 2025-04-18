#include "stm32f10x.h"  // Device header

/**
  * 函 数：PWM_Init
  * 描 述：利用 TIM4 产生 PWM 信号，并输出到 PB7（TIM4_CH2）
  *        系统假定时钟72MHz，预分频器PSC设为72-1（1µs时基），自动重装载值ARR设为20000-1（20ms周期）
  */
void PWM_Init(void)
{
    /* 开启TIM4和GPIOB的时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 配置PB7为复用推挽输出（TIM4_CH2默认映射到PB7） */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 选择TIM4的内部时钟 */
    TIM_InternalClockConfig(TIM4);

    /* 时基单元初始化 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;    // 20ms周期
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;      // 1µs时基
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

    /* 输出比较初始化 – 使用 TIM4 通道2 */
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);              // 初始化结构体所有成员
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;      // PWM模式1
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 高电平有效
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                   // 初始脉宽为0µs
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);             // 配置通道2

    /* 启动TIM4 */
    TIM_Cmd(TIM4, ENABLE);
}

/**
  * 函 数：PWM_SetCompare2
  * 描 述：设置 TIM4_CH2 的比较匹配值，从而改变PWM脉宽
  * 参    数：Compare - 脉宽值，单位为µs
  */
void PWM_SetCompare2(uint16_t Compare)
{
    TIM_SetCompare2(TIM4, Compare);
}
