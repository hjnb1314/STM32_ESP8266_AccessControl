#include "stm32f10x.h"
#include "Serial.h"
#include "Key.h"
#include "OLED.h"

/*
 * 请确认 Serial_Init() 中的波特率设置为 115200，
 * 例如在 Serial.c 中：
 *    USART_InitStructure.USART_BaudRate = 115200;
 */

int main(void)
{
    uint8_t toggleState = 0;    // 内部状态：0 表示 false，1 表示 true

    // 初始化按键、串口和 OLED
    Key_Init();
    Serial_Init();
    OLED_Init();
    OLED_Clear();

    while (1)
    {
		
		OLED_ShowString(1, 1, "Key:");
        // --- 1. 按键检测与数据发送 ---
        if (Key_GetNum())
        {
            // 简单延时去抖，可根据实际情况调整该延时
            volatile uint32_t delay = 50000;
            while (delay--);

            // 切换状态：0 <-> 1
            toggleState = (toggleState == 0) ? 1 : 0;

            // 发送字符 '0' 或 '1'
            Serial_SendByte(toggleState + '0');
        }

        // --- 2. 接收来自 ESP8266 下发的数据，更新 OLED 显示 ---
        if (Serial_GetRxFlag() == 1)
        {
			uint8_t receivedData = Serial_GetRxData();  // 获取接收到的数据
            OLED_ShowChar(1, 5, receivedData);  // 在OLED的第1行第1列显示接收到的数据
        }
    }
}
