#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "usart3.h"
#include "as608.h"
#include "oled_iic.h"
#include "relay.h"
// �� ������ʹ�þ������ͷ�ļ� ��
#include "button4_4.h"

u32 AS608Addr = 0XFFFFFFFF; //Ĭ��
char str2[6] = {0};


//----------------------------------------------------------------------------
// ���������������ԭ����ͬ����ȥ���� #include "key.h" ������� key_num ������
//----------------------------------------------------------------------------

//���ڷ���һ���ֽ�
static void MYUSART_SendData(u8 data)
{
    while((USART3->SR & 0X40) == 0);
    USART3->DR = data;
}

//���Ͱ�ͷ
static void SendHead(void)
{
    MYUSART_SendData(0xEF);
    MYUSART_SendData(0x01);
}

//���͵�ַ
static void SendAddr(void)
{
    MYUSART_SendData(AS608Addr >> 24);
    MYUSART_SendData(AS608Addr >> 16);
    MYUSART_SendData(AS608Addr >> 8);
    MYUSART_SendData(AS608Addr);
}

//���Ͱ���ʶ
static void SendFlag(u8 flag)
{
    MYUSART_SendData(flag);
}

//���Ͱ�����
static void SendLength(int length)
{
    MYUSART_SendData(length >> 8);
    MYUSART_SendData(length);
}

//����ָ����
static void Sendcmd(u8 cmd)
{
    MYUSART_SendData(cmd);
}

//����У���
static void SendCheck(u16 check)
{
    MYUSART_SendData(check >> 8);
    MYUSART_SendData(check);
}

//�ж��жϽ��յ�������û��Ӧ���
static u8 *JudgeStr(u16 waittime)
{
    char *data;
    u8 str[8];
    str[0] = 0xef;
    str[1] = 0x01;
    str[2] = AS608Addr >> 24;
    str[3] = AS608Addr >> 16;
    str[4] = AS608Addr >> 8;
    str[5] = AS608Addr;
    str[6] = 0x07;
    str[7] = '\0';
    USART3_RX_STA = 0;
    while (--waittime)
    {
        delay_ms(1);
        if(USART3_RX_STA & 0X8000) //���յ�һ������
        {
            USART3_RX_STA = 0;
            data = strstr((const char*)USART3_RX_BUF, (const char*)str);
            if(data) return (u8*)data;
        }
    }
    return 0;
}

//¼��ͼ�� PS_GetImage
u8 PS_GetImage(void)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x03);
    Sendcmd(0x01);
    temp = 0x01 + 0x03 + 0x01;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//�������� PS_GenChar
u8 PS_GenChar(u8 BufferID)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x04);
    Sendcmd(0x02);
    MYUSART_SendData(BufferID);
    temp = 0x01 + 0x04 + 0x02 + BufferID;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//��ȷ�ȶ���öָ������ PS_Match
u8 PS_Match(void)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x03);
    temp = 0x01 + 0x03 + 0x03;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//����ָ�� PS_Search
u8 PS_Search(u8 BufferID, u16 StartPage, u16 PageNum, SearchResult *p)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x08);
    Sendcmd(0x04);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(StartPage >> 8);
    MYUSART_SendData(StartPage);
    MYUSART_SendData(PageNum >> 8);
    MYUSART_SendData(PageNum);
    temp = 0x01 + 0x08 + 0x04 + BufferID
           + (StartPage >> 8) + (u8)StartPage
           + (PageNum >> 8) + (u8)PageNum;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data)
    {
        ensure = data[9];
        p->pageID   = (data[10] << 8) + data[11];
        p->mathscore = (data[12] << 8) + data[13];
    }
    else ensure = 0xff;
    return ensure;
}

//�ϲ����� PS_RegModel
u8 PS_RegModel(void)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x05);
    temp = 0x01 + 0x03 + 0x05;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//����ģ�� PS_StoreChar
u8 PS_StoreChar(u8 BufferID, u16 PageID)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x06);
    Sendcmd(0x06);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(PageID >> 8);
    MYUSART_SendData(PageID);
    temp = 0x01 + 0x06 + 0x06 + BufferID
           + (PageID >> 8) + (u8)PageID;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//ɾ��ģ�� PS_DeletChar
u8 PS_DeletChar(u16 PageID, u16 N)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x07);
    Sendcmd(0x0C);
    MYUSART_SendData(PageID >> 8);
    MYUSART_SendData(PageID);
    MYUSART_SendData(N >> 8);
    MYUSART_SendData(N);
    temp = 0x01 + 0x07 + 0x0C
           + (PageID >> 8) + (u8)PageID
           + (N >> 8) + (u8)N;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//���ָ�ƿ� PS_Empty
u8 PS_Empty(void)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x0D);
    temp = 0x01 + 0x03 + 0x0D;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//дϵͳ�Ĵ��� PS_WriteReg
u8 PS_WriteReg(u8 RegNum, u8 DATA)
{
    u16 temp = 0;
    u8  ensure;
    u8  *rdata;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x05);
    Sendcmd(0x0E);
    MYUSART_SendData(RegNum);
    MYUSART_SendData(DATA);
    temp = RegNum + DATA + 0x01 + 0x05 + 0x0E;
    SendCheck(temp);
    rdata = JudgeStr(2000);
    if(rdata) ensure = rdata[9];
    else      ensure = 0xff;
    if(ensure == 0) printf("\r\n���ò����ɹ���");
    else            printf("\r\n%s", EnsureMessage(ensure));
    return ensure;
}

//��ϵͳ�������� PS_ReadSysPara
u8 PS_ReadSysPara(SysPara *p)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x0F);
    temp = 0x01 + 0x03 + 0x0F;
    SendCheck(temp);
    data = JudgeStr(1000);
    if(data)
    {
        ensure = data[9];
        p->PS_max = (data[14] << 8) + data[15];
        p->PS_level = data[17];
        p->PS_addr  = (data[18] << 24) + (data[19] << 16) + (data[20] << 8) + data[21];
        p->PS_size  = data[23];
        p->PS_N     = data[25];
    }
    else ensure = 0xff;

    if(ensure == 0x00)
    {
        printf("\r\nģ�����ָ������=%d", p->PS_max);
        printf("\r\n�Աȵȼ�=%d", p->PS_level);
        printf("\r\n��ַ=%x", p->PS_addr);
        printf("\r\n������=%d", p->PS_N * 9600);
    }
    else printf("\r\n%s", EnsureMessage(ensure));
    return ensure;
}

//����ģ���ַ PS_SetAddr
u8 PS_SetAddr(u32 PS_addr)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x07);
    Sendcmd(0x15);
    MYUSART_SendData(PS_addr >> 24);
    MYUSART_SendData(PS_addr >> 16);
    MYUSART_SendData(PS_addr >> 8);
    MYUSART_SendData(PS_addr);
    temp = 0x01 + 0x07 + 0x15
         + (u8)(PS_addr >> 24) + (u8)(PS_addr >> 16)
         + (u8)(PS_addr >> 8) + (u8)PS_addr;
    SendCheck(temp);
    AS608Addr = PS_addr;
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;

    AS608Addr = PS_addr;
    if(ensure == 0x00) printf("\r\n���õ�ַ�ɹ���");
    else               printf("\r\n%s", EnsureMessage(ensure));
    return ensure;
}

//д���±� PS_WriteNotepad
u8 PS_WriteNotepad(u8 NotePageNum, u8 *Byte32)
{
    u16 temp = 0;
    u8  ensure, i;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(36);
    Sendcmd(0x18);
    MYUSART_SendData(NotePageNum);
    for(i = 0; i < 32; i++)
    {
        MYUSART_SendData(Byte32[i]);
        temp += Byte32[i];
    }
    temp = 0x01 + 36 + 0x18 + NotePageNum + temp;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//�����±� PS_ReadNotepad
u8 PS_ReadNotepad(u8 NotePageNum, u8 *Byte32)
{
    u16 temp;
    u8  ensure, i;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x04);
    Sendcmd(0x19);
    MYUSART_SendData(NotePageNum);
    temp = 0x01 + 0x04 + 0x19 + NotePageNum;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data)
    {
        ensure = data[9];
        for(i = 0; i < 32; i++)
        {
            Byte32[i] = data[10 + i];
        }
    }
    else ensure = 0xff;
    return ensure;
}

//�������� PS_HighSpeedSearch
u8 PS_HighSpeedSearch(u8 BufferID, u16 StartPage, u16 PageNum, SearchResult *p)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x08);
    Sendcmd(0x1b);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(StartPage >> 8);
    MYUSART_SendData(StartPage);
    MYUSART_SendData(PageNum >> 8);
    MYUSART_SendData(PageNum);
    temp = 0x01 + 0x08 + 0x1b + BufferID
         + (StartPage >> 8) + (u8)StartPage
         + (PageNum >> 8) + (u8)PageNum;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data)
    {
        ensure = data[9];
        p->pageID   = (data[10] << 8) + data[11];
        p->mathscore= (data[12] << 8) + data[13];
    }
    else ensure = 0xff;
    return ensure;
}

//����Чģ����� PS_ValidTempleteNum
u8 PS_ValidTempleteNum(u16 *ValidN)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x1d);
    temp = 0x01 + 0x03 + 0x1d;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data)
    {
        ensure = data[9];
        *ValidN = (data[10] << 8) + data[11];
    }
    else ensure = 0xff;

    if(ensure == 0x00)
    {
        printf("\r\n��Чָ�Ƹ���=%d", (data[10] << 8) + data[11]);
    }
    else
    {
        printf("\r\n%s", EnsureMessage(ensure));
    }
    return ensure;
}

//��AS608���� PS_HandShake
u8 PS_HandShake(u32 *PS_Addr)
{
    SendHead();
    SendAddr();
    MYUSART_SendData(0X01);
    MYUSART_SendData(0X00);
    MYUSART_SendData(0X00);
    delay_ms(200);
    if(USART3_RX_STA & 0X8000)
    {
        if(USART3_RX_BUF[0] == 0XEF
           && USART3_RX_BUF[1] == 0X01
           && USART3_RX_BUF[6] == 0X07)
        {
            *PS_Addr = (USART3_RX_BUF[2] << 24) + (USART3_RX_BUF[3] << 16)
                     + (USART3_RX_BUF[4] << 8)  +  USART3_RX_BUF[5];
            USART3_RX_STA = 0;
            return 0;
        }
        USART3_RX_STA = 0;
    }
    return 1;
}

//ģ��Ӧ���ȷ������Ϣ����
const char *EnsureMessage(u8 ensure)
{
    const char *p;
    switch(ensure)
    {
    case 0x00: p="OK";break;
    case 0x01: p="���ݰ����մ���";break;
    case 0x02: p="��������û����ָ";break;
    case 0x03: p="¼��ָ��ͼ��ʧ��";break;
    case 0x04: p="ָ��̫�ɻ�̫��";break;
    case 0x05: p="ָ��̫ʪ��̫��";break;
    case 0x06: p="ָ��ͼ��̫��";break;
    case 0x07: p="ָ��������̫��";break;
    case 0x08: p="ָ�Ʋ�ƥ��";break;
    case 0x09: p="û��������ָ��";break;
    case 0x0a: p="�����ϲ�ʧ��";break;
    case 0x0b: p="��ַ��ų�����Χ";break;
    case 0x10: p="ɾ��ģ��ʧ��";break;
    case 0x11: p="���ָ�ƿ�ʧ��";break;
    case 0x15: p="������������Чͼ";break;
    case 0x18: p="��дFLASH����";break;
    case 0x19: p="δ�������";break;
    case 0x1a: p="��Ч�Ĵ�����";break;
    case 0x1b: p="�Ĵ������ݴ���";break;
    case 0x1c: p="���±�ҳ�����";break;
    case 0x1f: p="ָ�ƿ���";break;
    case 0x20: p="��ַ����";break;
    default:   p="����ȷ��������";break;
    }
    return p;
}

//��ʾȷ���������Ϣ(����OLED����ʾ���ٷ�װ�����������)
void ShowErrMessage(u8 ensure)
{
    // ���Ը�����Ҫ���� OLED_ShowCH(...) ��ʾ������ʾ
    // ���������
}

//----------------------------------------------------------------------------
// �� �ص㣺���� 3 ������ԭ���õ��� KEY_Scan(0) �� key_num���ָ��� Button4_4_Scan() ��
//----------------------------------------------------------------------------

//¼ָ��
void Add_FR(void)
{
    u8 i=0, ensure, processnum = 0;
    u8 ID_NUM = 0;

    while(1)
    {
        switch (processnum)
        {
        case 0:
            i++;
            OLED_ShowCH(0, 2, "    �밴��ָ    ");
            ensure = PS_GetImage();
            if(ensure == 0x00)
            {
                ensure = PS_GenChar(CharBuffer1); //��������
                if(ensure == 0x00)
                {
                    OLED_ShowCH(0, 2, "    ָ������    ");
                    OLED_ShowCH(0, 4, "                ");
                    i = 0;
                    processnum = 1; //�����ڶ���
                }
                else ShowErrMessage(ensure);
            }
            else ShowErrMessage(ensure);
            break;

        case 1:
            i++;
            OLED_ShowCH(0, 2, "   ���ٰ�һ��   ");
            OLED_ShowCH(0, 4, "                ");
            ensure = PS_GetImage();
            if(ensure == 0x00)
            {
                ensure = PS_GenChar(CharBuffer2);
                if(ensure == 0x00)
                {
                    OLED_ShowCH(0, 2, "    ָ������    ");
                    OLED_ShowCH(0, 4, "                ");
                    i = 0;
                    processnum = 2;
                }
                else ShowErrMessage(ensure);
            }
            else ShowErrMessage(ensure);
            break;

        case 2:
            OLED_ShowCH(0, 2, "  �Ա�����ָ��  ");
            OLED_ShowCH(0, 4, "                ");
            ensure = PS_Match();
            if(ensure == 0x00)
            {
                OLED_ShowCH(0, 2, "    �Աȳɹ�    ");
                OLED_ShowCH(0, 4, "                ");
                processnum = 3;
            }
            else
            {
                OLED_ShowCH(0, 2, "    �Ա�ʧ��    ");
                OLED_ShowCH(0, 4, "                ");
                ShowErrMessage(ensure);
                i = 0;
                processnum = 0;
            }
            delay_ms(500);
            break;

        case 3:
            OLED_ShowCH(0, 2, "  ����ָ��ģ��  ");
            OLED_ShowCH(0, 4, "                ");
            delay_ms(500);
            ensure = PS_RegModel();
            if(ensure == 0x00)
            {
                OLED_ShowCH(0, 2, "����ָ��ģ��ɹ�");
                OLED_ShowCH(0, 4, "                ");
                processnum = 4;
            }
            else
            {
                processnum = 0;
                ShowErrMessage(ensure);
            }
            delay_ms(1000);
            break;

        case 4:
        {
            OLED_ShowCH(0, 0, " ��4�� ��2��    ");
            OLED_ShowCH(0, 2, " ��3����        ");
            OLED_ShowCH(0, 4, "  0<= ID <=99   ");
            while(1)
            {
                // ��ѯ�������
                u8 key_now = Button4_4_Scan();
                if(key_now == 2) //ԭ��key_num==2 => ��
                {
                    if(ID_NUM > 0) ID_NUM--;
                }
                else if(key_now == 4) //ԭ��key_num==4 => ��
                {
                    if(ID_NUM < 99) ID_NUM++;
                }
                else if(key_now == 3) //ԭ��key_num==3 => ����
                {
                    // �洢������ָ��ID
                    ensure = PS_StoreChar(CharBuffer2, ID_NUM);
                    if(ensure == 0x00)
                    {
                        OLED_Clear();
                        OLED_ShowCH(0, 2, "  ¼��ָ�Ƴɹ�  ");
                        delay_ms(1500);
                        OLED_Clear();
                        /*OLED_ShowCH(0, 0, "�ѵ�¼");
                        OLED_ShowCH(16, 2, "K1���ָ��");
                        OLED_ShowCH(16, 4, "K3ɾ��ָ��");
                        OLED_ShowCH(16, 6, "K15�˳���¼");*/
                        return ;
                    }
                    else
                    {
                        OLED_Clear();
                        ShowErrMessage(ensure);
                        processnum = 0;
                        break; //���� while(1)�����»ص� processnum=0
                    }
                }
                // ��ʾ ID
                OLED_ShowCH(40, 6, "ID=");
                OLED_ShowNum(65, 6, ID_NUM, 2, 1);
                delay_ms(100);
            }
            break;
        }
        } // switch

        delay_ms(400);
        if(i == 10) //����һ������û����ָ���˳�
        {
            break;
        }
    }
}

//ˢָ��(��֤)
int press_FR(void)
{
	char toggleState = 0;
	Relay_Init();
    SearchResult seach;
    u8 ensure;
    char str[20];
    OLED_Clear();
    OLED_ShowCH(32, 2, "�밴��ָ");
	int flag = 0;

    while(1)
    {
        // ��ȡ�������ֵ
        u8 key_now = Button4_4_Scan();
        // ԭ�߼���while(key_num != 1) => ���� KEY1(=1) �ͳ������
        // ����ֻҪɨ�赽 1 ���˳�
        if(key_now == 1) // �ȼ��ھɵ� KEY1==0
        {
            break;
        }

        ensure = PS_GetImage();
        if(ensure == 0x00) //��ȡͼ��ɹ�
        {
            ensure = PS_GenChar(CharBuffer1);
            if(ensure == 0x00) //���������ɹ�
            {
                ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
                if(ensure == 0x00) //�����ɹ�
                {
                    OLED_ShowCH(0, 2, "  ָ����֤�ɹ�  ");
                    sprintf(str, " ID:%d �÷�:%d ", seach.pageID, seach.mathscore);
                    OLED_ShowCH(0, 4, (u8*)str);
					
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
					
					flag = 1;
					return flag;
                }
                else
                {
                    OLED_ShowCH(32, 2, "��֤ʧ��");
                    delay_ms(1500);
                }
            }
            OLED_Clear();
            OLED_ShowCH(32, 2, "�밴��ָ");
        }
        delay_ms(50);
    }


}

//ɾ��ָ��
void Del_FR(void)
{
    u8  ensure;
    u16 ID_NUM = 0;
    OLED_ShowCH(0, 0, "4��2�� 3ȷ��  ");
    OLED_ShowCH(0, 2, " K5���ָ�ƿ� ");
    OLED_ShowCH(0, 4, "1���� 0<=ID<=99");
    while(1)
    {
        u8 key_now = Button4_4_Scan();
        if(key_now == 2) //��
        {
            if(ID_NUM > 0) ID_NUM--;
        }
        else if(key_now == 4) //��
        {
            if(ID_NUM < 99) ID_NUM++;
        }
        else if(key_now == 1) //�൱��ԭ��KEY1����
        {
            goto MENU;
        }
        else if(key_now == 5) //���ָ�ƿ�
        {
            ensure = PS_Empty();
            if(ensure == 0)
            {
                OLED_Clear();
                OLED_ShowCH(0, 2, " ���ָ�ƿ�ɹ� ");
            }
            else ShowErrMessage(ensure);
            delay_ms(1500);
            goto MENU;
        }
        else if(key_now == 3) //ȷ��ɾ��ָ��ID
        {
            ensure = PS_DeletChar(ID_NUM, 1);
            if(ensure == 0)
            {
                OLED_Clear();
                OLED_ShowCH(0, 2, "  ɾ��ָ�Ƴɹ�  ");
            }
            else ShowErrMessage(ensure);
            delay_ms(1500);
            goto MENU;
        }
        //ʵʱ��ʾ ID
        OLED_ShowCH(40, 6, "ID=");
        OLED_ShowNum(65, 6, ID_NUM, 2, 1);
        delay_ms(100);
    }

MENU:
    OLED_Clear();
                OLED_ShowCH(16,0,"K14������");
                OLED_ShowCH(16,2,"K1��ָ��");
                OLED_ShowCH(16,4,"K3ɾָ��");
                OLED_ShowCH(16,6,"K15�˳�������");
}
