#include "sys.h"
#include "rc522.h"
#include "is_card_authorized.h"
#include "delay.h"
#include "oled_iic.h"
#include "stdio.h"
#include "timer.h"
#include "as608.h"
#include "usart3.h"
#include "relay.h"
#include "button4_4.h"
#include "beep.h"
#include "Serial.h"
#include "stm32f10x_flash.h"
#include <string.h>

/* -------- Flash 保存区 -------- */
/* 64?KB 器件最后 1?KB；128?KB 的请改 0x0801F800 */
#define FLASH_PWD_ADDR  0x0800FC00U

uint8_t error_cishu = 0; //报警次数统计

extern unsigned char CT[2];
extern unsigned char SN[4];

unsigned char last_unauth_card[4] = {0}; // 保存上一张未授权卡号
int last_unauth_valid = 0;               // 标志位

/* ---------- Flash 读 / 写 6 位密码 ---------- */
static void flash_pwd_read(char *pwd)
{
    /* 低字节才是真正数据，高字节此前写入的是同样字符 */
    uint8_t first = *(uint8_t *)FLASH_PWD_ADDR;
    if (first < '0' || first > '9') {          /* 空 Flash，则给默认值 */
        memcpy(pwd, "123456", 6);
    } else {
        for (int i = 0; i < 6; i++)            /* 只取每个半字的低 8 位 */
            pwd[i] = *(uint8_t *)(FLASH_PWD_ADDR + i * 2);
    }
    pwd[6] = '\0';
}

static void flash_pwd_write(const char *pwd)
{
    FLASH_Unlock();
    FLASH_ErasePage(FLASH_PWD_ADDR);
    for (int i = 0; i < 6; i++) {
        /* 把同一个 ASCII 字节写到高/低 8 位，便于后续调试查看 */
        uint16_t half = ((uint16_t)pwd[i] << 8) | pwd[i];
        FLASH_ProgramHalfWord(FLASH_PWD_ADDR + i * 2, half);
    }
    FLASH_Lock();
}

/* ---------- 按键→数字 ---------- */
static char key_to_digit(int key)
{
    return (key >= 0 && key <= 9) ? ('0' + key) : 'X';
}

/* 读取 6 位数字，边输边显示 ‘*’ */
static void get_6digits(char *buf)
{
    uint8_t idx = 0;
    for (int i = 0; i < 6; i++) OLED_ShowCH(i * 8, 2, " ");

    while (idx < 6)
    {
        int k;
        do { k = Button4_4_Scan(); } while (k == 0);      /* 等有键 */

        char d = key_to_digit(k);
        if (d != 'X') {
            buf[idx++] = d;
            OLED_ShowCH((idx - 1) * 8, 2, "*");
        }
        while (Button4_4_Scan() != 0);                    /* 等松键 */
        delay_ms(20);
    }
    buf[6] = '\0';
}

/* 密码登录 */
static int password_login(void)
{
	char toggleState = 0;
    char stored[7], entered[7];
    flash_pwd_read(stored);

    OLED_Clear();
    OLED_ShowCH(0, 0, "输入6位密码");
	OLED_ShowCH(0, 6, "输入次数:");
	OLED_ShowNum(78, 6, error_cishu, 1,1);
    get_6digits(entered);

    if (strcmp(stored, entered) == 0) {
        OLED_ShowCH(0, 4, "密码正确"); 
		delay_ms(800);
		
		
		//模拟开锁
		OLED_Clear();
		OLED_ShowCH(0, 0, "正在开锁...");
		delay_ms(5000);
		Serial_SendByte(toggleState + '1');
		 
		while(1){
		if (Serial_GetRxFlag() == 1&&Serial_GetRxData()=='1')
        {
		//Servo_SetAngle(180.0);
		Relay_On();
		OLED_Clear();
		OLED_ShowCH(0, 0, "开锁成功.");
		delay_ms(5000);
		break;
        }
	}
		
		return 1;
    } else {
        OLED_ShowCH(0, 4, "密码错误"); 
		if(error_cishu<2){
			error_cishu++;
			
		}else{
			error_cishu = 0;
			//报警逻辑
			OLED_Clear();
			OLED_ShowCH(16, 2, "锁定报警中...");
			BEEP_On();
			for(int temp=0;temp<15;temp++)delay_ms(5000);
			BEEP_Off();
		}
		delay_ms(800); 
		OLED_Clear();
		OLED_ShowCH(0,0,"未登录");
        OLED_ShowCH(16,2,"K16指纹验证");
        OLED_ShowCH(16,4,"K14密码验证");
		OLED_ShowCH(16,6,"K10card验证");
		return 0;
    }
}

/* 改密码 */
static void password_change(void)
{
    char p1[7], p2[7];
    OLED_Clear(); OLED_ShowCH(0,0,"新密码:"); get_6digits(p1);
    OLED_Clear(); OLED_ShowCH(0,0,"再次输入:"); get_6digits(p2);

    if (strcmp(p1, p2) == 0) {
        flash_pwd_write(p1); OLED_ShowCH(0,4,"修改成功");
    } else {
        OLED_ShowCH(0,4,"两次不一致");
    }
    delay_ms(800);
	OLED_Clear();
}


//刷指纹(验证)
int press_FR(uint8_t C)
{
	char toggleState = 0;
	Relay_Init();
	BEEP_Init();
	//Servo_Init();
    SearchResult seach;
    u8 ensure;
    char str[20];
    OLED_Clear();
    OLED_ShowCH(32, 2, "请按手指");
	int flag = 0;

    while(1)
    {
        // 获取矩阵键盘值
        u8 key_now = Button4_4_Scan();
        // 原逻辑：while(key_num != 1) => 不按 KEY1(=1) 就持续检测
        // 所以只要扫描到 1 就退出
        if(key_now == 1) // 等价于旧的 KEY1==0
        {
			return flag;
            break;
        }

        ensure = PS_GetImage();
        if(ensure == 0x00) //获取图像成功
        {
            ensure = PS_GenChar(CharBuffer1);
            if(ensure == 0x00) //生成特征成功
            {
                ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
                if(ensure == 0x00) //搜索成功
                {
                    OLED_ShowCH(0, 2, "  指纹验证成功  ");
                    sprintf(str, " ID:%d 得分:%d ", seach.pageID, seach.mathscore);
                    OLED_ShowCH(0, 4, (u8*)str);
					
				//模拟开锁
				OLED_Clear();
				OLED_ShowCH(0, 0, "正在开锁...");
				delay_ms(5000);
				Serial_SendByte(toggleState + '1');
				 
			   while(1){
					if (Serial_GetRxFlag() == 1&&Serial_GetRxData()=='1')
					{
					//Servo_SetAngle(180.0);
					Relay_On();
					OLED_Clear();
					OLED_ShowCH(0, 0, "开锁成功.");
					delay_ms(5000);
					break;
					}
			}
					
					flag = 1;
					return flag;
                }
                else
                {
                    OLED_ShowCH(32, 4, "验证失败");
                    delay_ms(1500);
		if(error_cishu<2){
			error_cishu++;	
		}else{
			error_cishu = 0;
			//报警逻辑
			OLED_Clear();
            OLED_ShowCH(16, 2, "锁定报警中...");
			BEEP_On();
			for(int temp=0;temp<15;temp++)delay_ms(5000);
			BEEP_Off();
				OLED_Clear();
		OLED_ShowCH(0,0,"未登录");
        OLED_ShowCH(16,2,"K16指纹验证");
        OLED_ShowCH(16,4,"K14密码验证");
		OLED_ShowCH(16,6,"K10card验证");
			flag = 0;
			return flag;
			
		}
			delay_ms(800); 
                }
            }
            OLED_Clear();
            OLED_ShowCH(32, 2, "请按手指");
			OLED_ShowCH(32, 0, "按K1返回");
			OLED_ShowCH(0, 6, "输入次数:");
	        OLED_ShowNum(78, 6, error_cishu, 1,1);
        }
        delay_ms(50);
    }


}

/* ---------------- 主函数 ---------------- */
int main(void)
{
    extern const u8 BMP1[];
    HZ = GB16_NUM();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();
	usart3_init(57600);
    Relay_Init();
	BEEP_Init();
	Button4_4_Init();
	OLED_Init();
	OLED_Clear();
	Serial_Init();
	RC522_Init();
	//Servo_Init();
	//Servo_SetAngle(0.0);
	
	 // 先释放JTAG
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 禁用JTAG，只保留SWD
	
    while (1)
    {
		uint8_t toggleState = 0;
		OLED_ShowCH(0,0,"未登录");
        OLED_ShowCH(16,2,"K16指纹验证");
        OLED_ShowCH(16,4,"K14密码验证");
		OLED_ShowCH(16,6,"K10card验证");

        uint8_t key; int logged = 0;
        while (!logged) {
            key = Button4_4_Scan();
            if (key == 16) 
			{ 
			OLED_Clear(); 
			if (press_FR(error_cishu)) {
			logged = 1; 
			}else{
					OLED_Clear();
					OLED_ShowCH(0,0,"未登录");
					OLED_ShowCH(16,2,"K16指纹验证");
					OLED_ShowCH(16,4,"K14密码验证");
					OLED_ShowCH(16,6,"K10card验证");
					logged = 0;
				}
			}
            else if (key == 14) { if (password_login()) logged = 1; }
			else if (key == 10) { 

					int ret = is_authorized_card(); // OLED内部自动显示 Unlock OK/Access Denied
					delay_ms(5000);
					if(ret)
					{
					//模拟开锁
					OLED_Clear();
					OLED_ShowCH(0, 0, "正在开锁...");
					delay_ms(5000);
					Serial_SendByte(toggleState + '1');
					 
					while(1){
					if (Serial_GetRxFlag() == 1&&Serial_GetRxData()=='1')
					{
					//Servo_SetAngle(180.0);
					Relay_On();
					OLED_Clear();
					OLED_ShowCH(0, 0, "开锁成功.");
					delay_ms(5000);
					break;
					}
						
					logged = 1;
					}
				}
					else {
					OLED_Clear();
					OLED_ShowCH(0,0,"未登录");
					OLED_ShowCH(16,2,"K16指纹验证");
					OLED_ShowCH(16,4,"K14密码验证");
					OLED_ShowCH(16,6,"K10card验证");
					logged = 0;
					}
			}
        }

        OLED_Clear();
        OLED_ShowCH(16,0,"K14改密码");
        OLED_ShowCH(16,2,"K1/3添/删指纹");
        OLED_ShowCH(16,4,"K8/9添/删card");
        OLED_ShowCH(16,6,"K15退出并关锁");

        while (logged) {
            key = Button4_4_Scan();
            if (key == 1){ OLED_Clear(); Add_FR(); }
            else if (key == 3){ OLED_Clear(); Del_FR(); }
            else if (key == 14){ password_change(); }
			else if (key == 8){	
			add_card_mode();
			OLED_Clear();
			}
			else if (key == 9){	
			delete_card_mode();
			OLED_Clear();
			}
            else if (key == 15){ 
			logged = 0; 
			OLED_Clear(); 
			//模拟关锁
				OLED_Clear();
				OLED_ShowCH(0, 0, "正在关锁...");
				delay_ms(5000);
				Serial_SendByte(toggleState + '0');
				 
			   while(1){
					if (Serial_GetRxFlag() == 1&&Serial_GetRxData()=='0')
					{
					//Servo_SetAngle(0.0);
					Relay_Off();
					OLED_Clear();
					OLED_ShowCH(0, 0, "关锁成功.");
					delay_ms(5000);
					break;
					}
			}
				
			
			
			OLED_Clear();
			}
            delay_ms(120);
            if (logged) {
        OLED_ShowCH(16,0,"K14改密码");
        OLED_ShowCH(16,2,"K1/3添/删指纹");
        OLED_ShowCH(16,4,"K8/9添/删card");
        OLED_ShowCH(16,6,"K15退出并关锁");
            }
        }
    }
}