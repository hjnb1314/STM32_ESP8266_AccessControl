#include "delay.h"
#include "usart3.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"

/* ---------- ȫ�ֱ������ֲ��� ---------- */
u8  USART3_RX_BUF[USART3_MAX_RECV_LEN];
u8  USART3_TX_BUF[USART3_MAX_SEND_LEN];
vu16 USART3_RX_STA = 0;

/* ����3 IRQ : �յ� 1 �ֽھ����� 10?ms ��ʱ�� */
void USART3_IRQHandler(void)
{
    u8 res;

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        res = USART_ReceiveData(USART3);

        if ((USART3_RX_STA & (1 << 15)) == 0)        // ��һ֡��δ������
        {
            if (USART3_RX_STA < USART3_MAX_RECV_LEN) // ������δ��
            {
                TIM_SetCounter(TIM2, 0);             // ����������

                if (USART3_RX_STA == 0)
                    TIM_Cmd(TIM2, ENABLE);           // �� 1 �ֽڣ����� 10?ms ��ʱ

                USART3_RX_BUF[USART3_RX_STA++] = res;
            }
            else
            {
                USART3_RX_STA |= 1 << 15;            // ǿ�ƽ��� (���)
            }
        }
    }
}

/* ���� 3 ��ʼ�� : ʹ�� TIM2 ��֡��ʱ */
void usart3_init(u32 bound)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    /* ����ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    USART_DeInit(USART3);

    /* PB10 ���� TX */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* PB11 ���� RX */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* USART ���� */
    USART_InitStructure.USART_BaudRate   = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;
    USART_InitStructure.USART_Parity     = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);      // ��������ж�
    USART_Cmd(USART3, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* ---- 10?ms ��ʱ��ʱ�� ---- */
    TIM2_Int_Init(99, 7199);   // (99+1)*(7199+1)/72?MHz �� 0.01?s
    TIM_Cmd(TIM2, DISABLE);    // �ȴ����ֽ�������
}

/* printf ��װ (���ֲ���) */
void u2_printf(char *fmt, ...)
{
    u16 i, j;
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char *)USART3_TX_BUF, fmt, ap);
    va_end(ap);
    i = strlen((const char *)USART3_TX_BUF);

    for (j = 0; j < i; j++)
    {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
        USART_SendData(USART3, USART3_TX_BUF[j]);
    }
}
