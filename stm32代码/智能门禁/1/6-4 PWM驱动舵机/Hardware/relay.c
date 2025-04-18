#include "relay.h"

// 初始化连接在PB12的继电器
void Relay_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    
    GPIO_InitTypeDef itd;
    itd.GPIO_Mode=GPIO_Mode_Out_PP;                     //推挽输出
    itd.GPIO_Pin=GPIO_Pin_12;      //使用Pin_12号引脚
    itd.GPIO_Speed=GPIO_Speed_2MHz;                     
    GPIO_Init(GPIOB,&itd);

	
    // 默认关闭继电器
    Relay_Off();
}

// 打开连接在PB12的继电器
void Relay_On(void) {
     GPIO_WriteBit(GPIOB,GPIO_Pin_12,0);
}

// 关闭连接在PB12的继电器
void Relay_Off(void) {
     GPIO_WriteBit(GPIOB,GPIO_Pin_12,1);
}
