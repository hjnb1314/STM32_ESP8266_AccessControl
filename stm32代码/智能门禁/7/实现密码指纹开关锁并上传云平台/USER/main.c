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
#include "Serial.h"
#include "stm32f10x_flash.h"
#include <string.h>

/* -------- Flash ������ -------- */
/* 64?KB ������� 1?KB��128?KB ����� 0x0801F800 */
#define FLASH_PWD_ADDR  0x0800FC00U

/* ---------- Flash �� / д 6 λ���� ---------- */
static void flash_pwd_read(char *pwd)
{
    /* ���ֽڲ����������ݣ����ֽڴ�ǰд�����ͬ���ַ� */
    uint8_t first = *(uint8_t *)FLASH_PWD_ADDR;
    if (first < '0' || first > '9') {          /* �� Flash�����Ĭ��ֵ */
        memcpy(pwd, "123456", 6);
    } else {
        for (int i = 0; i < 6; i++)            /* ֻȡÿ�����ֵĵ� 8 λ */
            pwd[i] = *(uint8_t *)(FLASH_PWD_ADDR + i * 2);
    }
    pwd[6] = '\0';
}

static void flash_pwd_write(const char *pwd)
{
    FLASH_Unlock();
    FLASH_ErasePage(FLASH_PWD_ADDR);
    for (int i = 0; i < 6; i++) {
        /* ��ͬһ�� ASCII �ֽ�д����/�� 8 λ�����ں������Բ鿴 */
        uint16_t half = ((uint16_t)pwd[i] << 8) | pwd[i];
        FLASH_ProgramHalfWord(FLASH_PWD_ADDR + i * 2, half);
    }
    FLASH_Lock();
}

/* ---------- ���������� ---------- */
static char key_to_digit(int key)
{
    return (key >= 0 && key <= 9) ? ('0' + key) : 'X';
}

/* ��ȡ 6 λ���֣��������ʾ ��*�� */
static void get_6digits(char *buf)
{
    uint8_t idx = 0;
    for (int i = 0; i < 6; i++) OLED_ShowCH(i * 8, 2, " ");

    while (idx < 6)
    {
        int k;
        do { k = Button4_4_Scan(); } while (k == 0);      /* ���м� */

        char d = key_to_digit(k);
        if (d != 'X') {
            buf[idx++] = d;
            OLED_ShowCH((idx - 1) * 8, 2, "*");
        }
        while (Button4_4_Scan() != 0);                    /* ���ɼ� */
        delay_ms(20);
    }
    buf[6] = '\0';
}

/* �����¼ */
static int password_login(void)
{
	char toggleState = 0;
    char stored[7], entered[7];
    flash_pwd_read(stored);

    OLED_Clear();
    OLED_ShowCH(0, 0, "����6λ����");
    get_6digits(entered);

    if (strcmp(stored, entered) == 0) {
        OLED_ShowCH(0, 4, "������ȷ"); 
		delay_ms(800);
		
		
		//ģ�⿪��
		OLED_Clear();
		OLED_ShowCH(0, 0, "���ڿ���...");
		delay_ms(5000);
		Serial_SendByte(toggleState + '1');
		 
		while(1){
		if (Serial_GetRxFlag() == 1&&Serial_GetRxData()=='1')
        {
		Relay_On();
		OLED_Clear();
		OLED_ShowCH(0, 0, "�����ɹ�.");
		delay_ms(5000);
		break;
        }
	}
		
		return 1;
    } else {
        OLED_ShowCH(0, 4, "�������"); delay_ms(800); return 0;
    }
}

/* ������ */
static void password_change(void)
{
    char p1[7], p2[7];
    OLED_Clear(); OLED_ShowCH(0,0,"������:"); get_6digits(p1);
    OLED_Clear(); OLED_ShowCH(0,0,"�ٴ�����:"); get_6digits(p2);

    if (strcmp(p1, p2) == 0) {
        flash_pwd_write(p1); OLED_ShowCH(0,4,"�޸ĳɹ�");
    } else {
        OLED_ShowCH(0,4,"���β�һ��");
    }
    delay_ms(800);
	OLED_Clear();
}

/* ---------------- ������ ---------------- */
int main(void)
{
    extern const u8 BMP1[];
    HZ = GB16_NUM();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();
	usart3_init(57600);
    Relay_Init();
	Button4_4_Init();
	OLED_Init();
	OLED_Clear();
	Serial_Init();

    while (1)
    {
		uint8_t toggleState = 0;
        OLED_ShowCH(0,0,"δ��¼");
        OLED_ShowCH(16,2,"K16ָ����֤");
        OLED_ShowCH(16,4,"K14������֤");

        uint8_t key; int logged = 0;
        while (!logged) {
            key = Button4_4_Scan();
            if (key == 16) { OLED_Clear(); if (press_FR()) logged = 1; }
            else if (key == 14) { if (password_login()) logged = 1; }
        }

        OLED_Clear();
        OLED_ShowCH(16,0,"K14������");
        OLED_ShowCH(16,2,"K1��ָ��");
        OLED_ShowCH(16,4,"K3ɾָ��");
        OLED_ShowCH(16,6,"K15�˳�������");

        while (logged) {
            key = Button4_4_Scan();
            if (key == 1){ OLED_Clear(); Add_FR(); }
            else if (key == 3){ OLED_Clear(); Del_FR(); }
            else if (key == 14){ password_change(); }
            else if (key == 15){ 
			logged = 0; 
			OLED_Clear(); 
			//ģ�����
				OLED_Clear();
				OLED_ShowCH(0, 0, "���ڹ���...");
				delay_ms(5000);
				Serial_SendByte(toggleState + '0');
				 
			   while(1){
					if (Serial_GetRxFlag() == 1&&Serial_GetRxData()=='0')
					{
					Relay_Off();
					OLED_Clear();
					OLED_ShowCH(0, 0, "�����ɹ�.");
					delay_ms(5000);
					break;
					}
			}
				
			
			
			OLED_Clear();
			}
            delay_ms(120);
            if (logged) {
                OLED_ShowCH(16,0,"K14������");
                OLED_ShowCH(16,2,"K1��ָ��");
                OLED_ShowCH(16,4,"K3ɾָ��");
                OLED_ShowCH(16,6,"K15�˳�������");
            }
        }
    }
}
