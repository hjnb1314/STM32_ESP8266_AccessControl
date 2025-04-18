#include "stm32f10x.h"  // Device header
#include "PWM.h"
#include "Servo.h"

/**
  * 函 数：Servo_Init
  * 描 述：舵机初始化，底层调用PWM初始化（PWM信号从PB7输出）
  */
void Servo_Init(void)
{
    PWM_Init();    // 初始化PWM模块
}

/**
  * 函 数：舵机设置角度
  * 描 述：将角度线性转换为PWM脉宽，映射规则：0°→500µs, 180°→2500µs
  * 参    数：Angle – 要设置的角度，取值范围0～180
  */
void Servo_SetAngle(float Angle)
{
    /* 限制角度在0~180之间 */
    if(Angle < 0.0f)
        Angle = 0.0f;
    if(Angle > 180.0f)
        Angle = 180.0f;
    
    /* 计算脉宽：pulse = (Angle/180)*2000 + 500 */
    uint16_t pulseWidth = (uint16_t)((Angle / 180.0f) * 2000.0f + 500.0f);
    PWM_SetCompare2(pulseWidth);
}
