#include "stm32f10x.h"
#include "Serial.h"
#include "OLED.h"

int main(void)
{
    uint8_t receivedValue = 0;

    // 初始化OLED显示和串口通信
    OLED_Init();
    OLED_Clear();
    Serial_Init();

    while(1)
    {
        // 检查是否接收到数据
        if (Serial_GetRxFlag())
        {
            receivedValue = Serial_GetRxData();
            OLED_Clear();
            // 在OLED第0行第0列显示接收到的数字（显示1位数字）
            OLED_ShowNum(1, 1, receivedValue - '0', 1);
        }
    }
}
