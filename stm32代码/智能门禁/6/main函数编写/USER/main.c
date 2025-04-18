#include "sys.h"
#include "delay.h"
#include "oled_iic.h"
#include "stdio.h"
#include "timer.h"
#include "as608.h"
#include "usart3.h"
#include "relay.h"
#include "button4_4.h"
#include "Servo.h"

int main(void)
{		
    extern const u8 BMP1[];
    HZ = GB16_NUM();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();
    usart3_init(57600);
	Relay_Init();

    // 1. 初始化矩阵键盘
    Button4_4_Init();

    // 2. 初始化 OLED
    OLED_Init();
   
	OLED_Clear();
    while(1)
    {
	OLED_ShowCH(0,0, "未登录");
	OLED_ShowCH(16,2,"k16键指纹验证");
	OLED_ShowCH(0,0, "K14键密码验证");
		uint8_t key_num = Button4_4_Scan();
		int key_c;
        // 扫描矩阵键盘，返回值范围 0~16
		 if(key_num == 16)      
        {
            OLED_Clear();
            key_c = press_FR();
			if(key_c==1){
		OLED_Clear();
		OLED_ShowCH(0,0, "K14改密码");
		OLED_ShowCH(16,2,"K1键添加指纹");
		OLED_ShowCH(16,4,"K3键删除指纹");
		OLED_ShowCH(16,6,"K15退出登录");
		while(key_num != 15){
			// 原有逻辑：添加、删除、验证指纹
			key_num = Button4_4_Scan();
			if(key_num == 1)      // K1：添加指纹
			{
				OLED_Clear();
				Add_FR();
				key_num=0;
				OLED_Clear();
			}
			else if(key_num == 3) // K3：删除指纹
			{
				OLED_Clear();
				Del_FR();
				key_num=0;
				OLED_Clear();
			}
			else if(key_num == 5) // K5：验证指纹
			{
				OLED_Clear();
				press_FR();
				key_num=0;
				OLED_Clear();
			}
			else if(key_num == 15) // K15：退出登录状态
			{
				OLED_Clear();
				key_num=0;
				OLED_Clear();
				break;
			}
			else if(key_num == 14) // K14：改密码
			{
				
			
			}
		OLED_ShowCH(0,0, "K14改密码");
		OLED_ShowCH(16,2,"K1键添加指纹");
		OLED_ShowCH(16,4,"K3键删除指纹");
		OLED_ShowCH(16,6,"K15退出登录");
			delay_ms(500); // 略作延时，避免过快扫描
				}
			} 
        }
		
		
		
    }
}
