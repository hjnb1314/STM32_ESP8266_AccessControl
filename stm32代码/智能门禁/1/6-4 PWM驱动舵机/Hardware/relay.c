#include "relay.h"

// ��ʼ��������PB12�ļ̵���
void Relay_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    
    GPIO_InitTypeDef itd;
    itd.GPIO_Mode=GPIO_Mode_Out_PP;                     //�������
    itd.GPIO_Pin=GPIO_Pin_12;      //ʹ��Pin_12������
    itd.GPIO_Speed=GPIO_Speed_2MHz;                     
    GPIO_Init(GPIOB,&itd);

	
    // Ĭ�Ϲرռ̵���
    Relay_Off();
}

// ��������PB12�ļ̵���
void Relay_On(void) {
     GPIO_WriteBit(GPIOB,GPIO_Pin_12,0);
}

// �ر�������PB12�ļ̵���
void Relay_Off(void) {
     GPIO_WriteBit(GPIOB,GPIO_Pin_12,1);
}
