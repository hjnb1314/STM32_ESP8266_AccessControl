#include "is_card_authorized.h"
#include "oled_iic.h"
#include "rc522.h"
#include "button4_4.h"
#include <string.h>

extern unsigned char CT[2];
extern unsigned char SN[4];

unsigned char card_list[MAX_CARDS][4] = {
    {0xD7, 0x24, 0x5B, 0x6A},
    // 其它自动初始化为0
};
unsigned char card_count = 1;

int is_authorized_card(void)
{
    OLED_Clear();
    OLED_ShowCH(0, 0, (u8 *)"Please card...");
    OLED_ShowCH(0, 6, (u8 *)"K15:EXIT");

    while (1)
    {
        // 检测按键K15退出
        int key = Button4_4_Scan();
        if (key == 15) {
            OLED_Clear();
            OLED_ShowCH(0, 0, (u8 *)"Exit card mode");
            delay_ms(5000);
            return 0; // 特殊返回值，表示退出
        }

        // 检测刷卡
        if (PcdRequest(PICC_REQALL, CT) == MI_OK)
        {
            if (PcdAnticoll(SN) == MI_OK)
            {
                // 比对卡号
                int found = 0;
                for (int i = 0; i < card_count; i++)
                {
                    if (memcmp(SN, card_list[i], 4) == 0)
                    {
                        OLED_Clear();
                        OLED_ShowCH(0, 0, (u8 *)"Unlock OK");
                        delay_ms(5000);
						return 1;
                        found = 1;
                        break;
                    }
                }
                if (!found)
                {
                    OLED_Clear();
                    OLED_ShowCH(0, 0, (u8 *)"Access Denied");
                    delay_ms(5000);
                }
                // 检测完后重新回到“Please card...”，继续等待下一个卡
                OLED_Clear();
                OLED_ShowCH(0, 0, (u8 *)"Please card...");
                OLED_ShowCH(0, 6, (u8 *)"K15:EXIT");
            }
        }
        delay_ms(50);
    }
}



// 带OLED提示的添加卡函数
int add_new_card(unsigned char *uid)
{
    int i;
    for (i = 0; i < card_count; i++)
    {
        if (memcmp(uid, card_list[i], 4) == 0)
        {
            OLED_Clear();
            OLED_ShowCH(0, 0, (u8 *)"Already Exists");
			delay_ms(5000);
            return 0; // 卡已存在
        }
    }

    if (card_count < MAX_CARDS)
    {
        memcpy(card_list[card_count], uid, 4);
        card_count++;
        OLED_Clear();
        OLED_ShowCH(0, 0, (u8 *)"Card Added!");
		delay_ms(5000);
        return 1; // 添加成功
    }
    else
    {
        OLED_Clear();
        OLED_ShowCH(0, 0, (u8 *)"Card List Full");
		delay_ms(5000);
        return -1; // 卡库已满
    }
}

void add_card_mode(void)
{
    OLED_Clear();
    OLED_ShowCH(0, 0, (u8 *)"Add card mode");
    OLED_ShowCH(0, 2, (u8 *)"Swipe card...");
    OLED_ShowCH(0, 6, (u8 *)"K15:EXIT");

    while (1)
    {
        int k = Button4_4_Scan();
        if (k == 15) {
            OLED_Clear();
            OLED_ShowCH(0, 0, (u8 *)"Exit add mode");
            delay_ms(1000);
            break; // 退出录卡模式
        }
        if (PcdRequest(PICC_REQALL, CT) == MI_OK)
        {
            if (PcdAnticoll(SN) == MI_OK)
            {
                int ret = add_new_card(SN); // OLED自动显示添加结果
                delay_ms(2000);
                OLED_Clear();
                OLED_ShowCH(0, 0, (u8 *)"Add card mode");
                OLED_ShowCH(0, 2, (u8 *)"Swipe card...");
                OLED_ShowCH(0, 6, (u8 *)"K15:EXIT");
            }
        }
        delay_ms(50);
    }
}

// 删除卡（带OLED提示），成功返回1，未找到返回0
int delete_card(unsigned char *uid)
{
    int i, found = 0;
    for (i = 0; i < card_count; i++)
    {
        if (memcmp(uid, card_list[i], 4) == 0)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        // 用最后一个卡覆盖被删的卡，实现数组收缩
        memcpy(card_list[i], card_list[card_count-1], 4);
        card_count--;
        OLED_Clear();
        OLED_ShowCH(0, 0, (u8 *)"Card Deleted!");
		delay_ms(5000);
        return 1;
    }
    else
    {
        OLED_Clear();
        OLED_ShowCH(0, 0, (u8 *)"Card Not Found");
		delay_ms(5000);
        return 0;
    }
}

// 删除卡模式：按K15退出，刷卡即删
void delete_card_mode(void)
{
    OLED_Clear();
    OLED_ShowCH(0, 0, (u8 *)"Delete card mode");
    OLED_ShowCH(0, 2, (u8 *)"Swipe card...");
    OLED_ShowCH(0, 6, (u8 *)"K15:EXIT");

    while (1)
    {
        int k = Button4_4_Scan();
        if (k == 15) {
            OLED_Clear();
            OLED_ShowCH(0, 0, (u8 *)"Exit del mode");
            delay_ms(1000);
            break;
        }
        if (PcdRequest(PICC_REQALL, CT) == MI_OK)
        {
            if (PcdAnticoll(SN) == MI_OK)
            {
                int ret = delete_card(SN); // OLED自动显示结果
                delay_ms(2000);
                OLED_Clear();
                OLED_ShowCH(0, 0, (u8 *)"Delete card mode");
                OLED_ShowCH(0, 2, (u8 *)"Swipe card...");
                OLED_ShowCH(0, 6, (u8 *)"K15:EXIT");
            }
        }
        delay_ms(50);
    }
}
