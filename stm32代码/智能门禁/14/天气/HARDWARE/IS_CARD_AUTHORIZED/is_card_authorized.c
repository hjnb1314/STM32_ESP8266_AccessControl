#include "is_card_authorized.h"
#include "oled_iic.h"
#include "rc522.h"
#include "button4_4.h"
#include <string.h>

extern unsigned char CT[2];
extern unsigned char SN[4];

unsigned char card_list[MAX_CARDS][4] = {
    {0xD7, 0x24, 0x5B, 0x6A},
    // �����Զ���ʼ��Ϊ0
};
unsigned char card_count = 1;

int is_authorized_card(void)
{
    OLED_Clear();
    OLED_ShowCH(0, 0, (u8 *)"Please card...");
    OLED_ShowCH(0, 6, (u8 *)"K15:EXIT");

    while (1)
    {
        // ��ⰴ��K15�˳�
        int key = Button4_4_Scan();
        if (key == 15) {
            OLED_Clear();
            OLED_ShowCH(0, 0, (u8 *)"Exit card mode");
            delay_ms(5000);
            return 0; // ���ⷵ��ֵ����ʾ�˳�
        }

        // ���ˢ��
        if (PcdRequest(PICC_REQALL, CT) == MI_OK)
        {
            if (PcdAnticoll(SN) == MI_OK)
            {
                // �ȶԿ���
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
                // ���������»ص���Please card...���������ȴ���һ����
                OLED_Clear();
                OLED_ShowCH(0, 0, (u8 *)"Please card...");
                OLED_ShowCH(0, 6, (u8 *)"K15:EXIT");
            }
        }
        delay_ms(50);
    }
}



// ��OLED��ʾ����ӿ�����
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
            return 0; // ���Ѵ���
        }
    }

    if (card_count < MAX_CARDS)
    {
        memcpy(card_list[card_count], uid, 4);
        card_count++;
        OLED_Clear();
        OLED_ShowCH(0, 0, (u8 *)"Card Added!");
		delay_ms(5000);
        return 1; // ��ӳɹ�
    }
    else
    {
        OLED_Clear();
        OLED_ShowCH(0, 0, (u8 *)"Card List Full");
		delay_ms(5000);
        return -1; // ��������
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
            break; // �˳�¼��ģʽ
        }
        if (PcdRequest(PICC_REQALL, CT) == MI_OK)
        {
            if (PcdAnticoll(SN) == MI_OK)
            {
                int ret = add_new_card(SN); // OLED�Զ���ʾ��ӽ��
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

// ɾ��������OLED��ʾ�����ɹ�����1��δ�ҵ�����0
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
        // �����һ�������Ǳ�ɾ�Ŀ���ʵ����������
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

// ɾ����ģʽ����K15�˳���ˢ����ɾ
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
                int ret = delete_card(SN); // OLED�Զ���ʾ���
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
