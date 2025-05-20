#include "stm32f10x.h"
#include "Servo.h"
#include "Delay.h"

int main(void)
{
    Servo_Init();

    while (1)
    {
        Servo_SetAngle(0);      // 转到0度
        Delay_ms(1000);
        Servo_SetAngle(90);     // 转到90度
        Delay_ms(1000);
        Servo_SetAngle(180);    // 转到180度
        Delay_ms(1000);
    }
}
