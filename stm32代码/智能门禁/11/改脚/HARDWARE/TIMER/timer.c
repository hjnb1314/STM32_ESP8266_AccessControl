#include "timer.h"
#include "key.h"
#include "usart3.h"

extern vu16 USART3_RX_STA;

/*-----------------------------------------------
 *  TIM2 —— 10?ms 帧超时定时器  (替代原 TIM4)
 *----------------------------------------------*/
void TIM2_Int_Init(u16 arr, u16 psc)
{
    NVIC_InitTypeDef       NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);      // 开 TIM2 时钟

    /* 基本计数器配置 : 计满即产生 Update 中断 */
    TIM_TimeBaseStructure.TIM_Period        = arr;            // 自动重装值
    TIM_TimeBaseStructure.TIM_Prescaler     = psc;            // 预分频
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                // 允许更新中断
    TIM_Cmd   (TIM2, ENABLE);                                 // 先启动，接收中断会再控制

    /* NVIC 配置 */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/* TIM2 中断服务 : 帧结束标记 */
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        USART3_RX_STA |= 1 << 15;          // 标记接收完成
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        TIM_Cmd(TIM2, DISABLE);            // 关闭定时器，等待下一帧重新开启
    }
}


/*
void TIM3_Int_Init(u16 arr, u16 psc)
{
    NVIC_InitTypeDef       NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period        = arr;
    TIM_TimeBaseStructure.TIM_Prescaler     = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
*/