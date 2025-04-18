#include "stm32f10x.h"  // Device header

/**
  * 函    数：PWM初始化 (使用PA0作为PWM输出)
  * 参    数：无
  * 返 回 值：无
  */
void PWM_Init(void)
{
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);         // 开启TIM2的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);          // 开启GPIOA的时钟

    /* GPIO初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;               // 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                     // 使用PA0
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);                        // 将PA0初始化为复用推挽输出

    /* 配置时钟源 */
    TIM_InternalClockConfig(TIM2);                                // 选择TIM2为内部时钟

    /* 时基单元初始化 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 不分频，滤波器时钟
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;               // 自动重装载值ARR（周期）
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;               // 预分频器PSC
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器（高级定时器才用）
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    /* 输出比较初始化（使用通道1对应PA0） */
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);                       // 结构体初始化

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;             // PWM模式1
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     // 输出极性为高
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能输出
    TIM_OCInitStructure.TIM_Pulse = 0;                            // 初始占空比值（CCR值）
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);                      // 使用通道1的配置（对应PA0）

    /* TIM使能 */
    TIM_Cmd(TIM2, ENABLE);                                        // 使能TIM2，定时器开始运行
}

/**
  * 函    数：PWM设置CCR (使用通道1)
  * 参    数：Compare 要写入的CCR的值，范围：0~(ARR+1)
  * 返 回 值：无
  * 注意事项：CCR和ARR共同决定占空比，此函数仅设置CCR的值
  *           占空比Duty = CCR / (ARR + 1)
  */
void PWM_SetCompare1(uint16_t Compare)
{
    TIM_SetCompare1(TIM2, Compare);  // 设置CCR1的值
}
