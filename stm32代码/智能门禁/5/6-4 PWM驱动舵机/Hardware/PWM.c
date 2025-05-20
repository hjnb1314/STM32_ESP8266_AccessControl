#include "stm32f10x.h"
#include "PWM.h"

void PWM_Init(void)
{
    // 1. 使能 TIM2 和 GPIOA 时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 2. 配置PA2为复用推挽输出（TIM2_CH3）
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;   // PA2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. TIM2定时器基本参数
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 20000-1;     // 20ms周期
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72-1;     // 1MHz计数(72MHz/72)
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    // 4. PWM输出模式配置（通道3）
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);     // 配置CH3

    // 5. 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

// 设置TIM2_CH3（PA2）PWM占空比
void PWM_SetCompare3(uint16_t Compare)
{
    TIM_SetCompare3(TIM2, Compare);
}
