#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "usart3.h"
#include "as608.h"
#include "oled_iic.h"
#include "relay.h"
// ★ 新增：使用矩阵键盘头文件 ★
#include "button4_4.h"

u32 AS608Addr = 0XFFFFFFFF; //默认
char str2[6] = {0};


//----------------------------------------------------------------------------
// 下面各函数内容与原来相同，仅去掉了 #include "key.h" 和里面对 key_num 的引用
//----------------------------------------------------------------------------

//串口发送一个字节
static void MYUSART_SendData(u8 data)
{
    while((USART3->SR & 0X40) == 0);
    USART3->DR = data;
}

//发送包头
static void SendHead(void)
{
    MYUSART_SendData(0xEF);
    MYUSART_SendData(0x01);
}

//发送地址
static void SendAddr(void)
{
    MYUSART_SendData(AS608Addr >> 24);
    MYUSART_SendData(AS608Addr >> 16);
    MYUSART_SendData(AS608Addr >> 8);
    MYUSART_SendData(AS608Addr);
}

//发送包标识
static void SendFlag(u8 flag)
{
    MYUSART_SendData(flag);
}

//发送包长度
static void SendLength(int length)
{
    MYUSART_SendData(length >> 8);
    MYUSART_SendData(length);
}

//发送指令码
static void Sendcmd(u8 cmd)
{
    MYUSART_SendData(cmd);
}

//发送校验和
static void SendCheck(u16 check)
{
    MYUSART_SendData(check >> 8);
    MYUSART_SendData(check);
}

//判断中断接收的数组有没有应答包
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
        if(USART3_RX_STA & 0X8000) //接收到一次数据
        {
            USART3_RX_STA = 0;
            data = strstr((const char*)USART3_RX_BUF, (const char*)str);
            if(data) return (u8*)data;
        }
    }
    return 0;
}

//录入图像 PS_GetImage
u8 PS_GetImage(void)
{
    u16 temp;
    u8  ensure;
    u8  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x03);
    Sendcmd(0x01);
    temp = 0x01 + 0x03 + 0x01;
    SendCheck(temp);
    data = JudgeStr(2000);
    if(data) ensure = data[9];
    else     ensure = 0xff;
    return ensure;
}

//生成特征 PS_GenChar
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

//精确比对两枚指纹特征 PS_Match
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

//搜索指纹 PS_Search
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

//合并特征 PS_RegModel
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

//储存模板 PS_StoreChar
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

//删除模板 PS_DeletChar
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

//清空指纹库 PS_Empty
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

//写系统寄存器 PS_WriteReg
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
    if(ensure == 0) printf("\r\n设置参数成功！");
    else            printf("\r\n%s", EnsureMessage(ensure));
    return ensure;
}

//读系统基本参数 PS_ReadSysPara
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
        printf("\r\n模块最大指纹容量=%d", p->PS_max);
        printf("\r\n对比等级=%d", p->PS_level);
        printf("\r\n地址=%x", p->PS_addr);
        printf("\r\n波特率=%d", p->PS_N * 9600);
    }
    else printf("\r\n%s", EnsureMessage(ensure));
    return ensure;
}

//设置模块地址 PS_SetAddr
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
    if(ensure == 0x00) printf("\r\n设置地址成功！");
    else               printf("\r\n%s", EnsureMessage(ensure));
    return ensure;
}

//写记事本 PS_WriteNotepad
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

//读记事本 PS_ReadNotepad
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

//高速搜索 PS_HighSpeedSearch
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

//读有效模板个数 PS_ValidTempleteNum
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
        printf("\r\n有效指纹个数=%d", (data[10] << 8) + data[11]);
    }
    else
    {
        printf("\r\n%s", EnsureMessage(ensure));
    }
    return ensure;
}

//与AS608握手 PS_HandShake
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

//模块应答包确认码信息解析
const char *EnsureMessage(u8 ensure)
{
    const char *p;
    switch(ensure)
    {
    case 0x00: p="OK";break;
    case 0x01: p="数据包接收错误";break;
    case 0x02: p="传感器上没有手指";break;
    case 0x03: p="录入指纹图像失败";break;
    case 0x04: p="指纹太干或太淡";break;
    case 0x05: p="指纹太湿或太糊";break;
    case 0x06: p="指纹图像太乱";break;
    case 0x07: p="指纹特征点太少";break;
    case 0x08: p="指纹不匹配";break;
    case 0x09: p="没有搜索到指纹";break;
    case 0x0a: p="特征合并失败";break;
    case 0x0b: p="地址序号超出范围";break;
    case 0x10: p="删除模板失败";break;
    case 0x11: p="清空指纹库失败";break;
    case 0x15: p="缓冲区内无有效图";break;
    case 0x18: p="读写FLASH出错";break;
    case 0x19: p="未定义错误";break;
    case 0x1a: p="无效寄存器号";break;
    case 0x1b: p="寄存器内容错误";break;
    case 0x1c: p="记事本页码错误";break;
    case 0x1f: p="指纹库满";break;
    case 0x20: p="地址错误";break;
    default:   p="返回确认码有误";break;
    }
    return p;
}

//显示确认码错误信息(若需OLED上显示可再封装，这里仅空着)
void ShowErrMessage(u8 ensure)
{
    // 可以根据需要，用 OLED_ShowCH(...) 显示错误提示
    // 这里仅留空
}

//----------------------------------------------------------------------------
// ★ 重点：下面 3 个函数原来用到了 KEY_Scan(0) 与 key_num，现改用 Button4_4_Scan() ★
//----------------------------------------------------------------------------

//录指纹
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
            OLED_ShowCH(0, 2, "    请按手指    ");
            ensure = PS_GetImage();
            if(ensure == 0x00)
            {
                ensure = PS_GenChar(CharBuffer1); //生成特征
                if(ensure == 0x00)
                {
                    OLED_ShowCH(0, 2, "    指纹正常    ");
                    OLED_ShowCH(0, 4, "                ");
                    i = 0;
                    processnum = 1; //跳到第二步
                }
                else ShowErrMessage(ensure);
            }
            else ShowErrMessage(ensure);
            break;

        case 1:
            i++;
            OLED_ShowCH(0, 2, "   请再按一次   ");
            OLED_ShowCH(0, 4, "                ");
            ensure = PS_GetImage();
            if(ensure == 0x00)
            {
                ensure = PS_GenChar(CharBuffer2);
                if(ensure == 0x00)
                {
                    OLED_ShowCH(0, 2, "    指纹正常    ");
                    OLED_ShowCH(0, 4, "                ");
                    i = 0;
                    processnum = 2;
                }
                else ShowErrMessage(ensure);
            }
            else ShowErrMessage(ensure);
            break;

        case 2:
            OLED_ShowCH(0, 2, "  对比两次指纹  ");
            OLED_ShowCH(0, 4, "                ");
            ensure = PS_Match();
            if(ensure == 0x00)
            {
                OLED_ShowCH(0, 2, "    对比成功    ");
                OLED_ShowCH(0, 4, "                ");
                processnum = 3;
            }
            else
            {
                OLED_ShowCH(0, 2, "    对比失败    ");
                OLED_ShowCH(0, 4, "                ");
                ShowErrMessage(ensure);
                i = 0;
                processnum = 0;
            }
            delay_ms(500);
            break;

        case 3:
            OLED_ShowCH(0, 2, "  生成指纹模板  ");
            OLED_ShowCH(0, 4, "                ");
            delay_ms(500);
            ensure = PS_RegModel();
            if(ensure == 0x00)
            {
                OLED_ShowCH(0, 2, "生成指纹模板成功");
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
            OLED_ShowCH(0, 0, " 按4加 按2减    ");
            OLED_ShowCH(0, 2, " 按3保存        ");
            OLED_ShowCH(0, 4, "  0<= ID <=99   ");
            while(1)
            {
                // 轮询矩阵键盘
                u8 key_now = Button4_4_Scan();
                if(key_now == 2) //原先key_num==2 => 减
                {
                    if(ID_NUM > 0) ID_NUM--;
                }
                else if(key_now == 4) //原先key_num==4 => 加
                {
                    if(ID_NUM < 99) ID_NUM++;
                }
                else if(key_now == 3) //原先key_num==3 => 保存
                {
                    // 存储特征到指定ID
                    ensure = PS_StoreChar(CharBuffer2, ID_NUM);
                    if(ensure == 0x00)
                    {
                        OLED_Clear();
                        OLED_ShowCH(0, 2, "  录入指纹成功  ");
                        delay_ms(1500);
                        OLED_Clear();
                        /*OLED_ShowCH(0, 0, "已登录");
                        OLED_ShowCH(16, 2, "K1添加指纹");
                        OLED_ShowCH(16, 4, "K3删除指纹");
                        OLED_ShowCH(16, 6, "K15退出登录");*/
                        return ;
                    }
                    else
                    {
                        OLED_Clear();
                        ShowErrMessage(ensure);
                        processnum = 0;
                        break; //跳出 while(1)，重新回到 processnum=0
                    }
                }
                // 显示 ID
                OLED_ShowCH(40, 6, "ID=");
                OLED_ShowNum(65, 6, ID_NUM, 2, 1);
                delay_ms(100);
            }
            break;
        }
        } // switch

        delay_ms(400);
        if(i == 10) //超过一定次数没按手指则退出
        {
            break;
        }
    }
}

//刷指纹(验证)
int press_FR(void)
{
	char toggleState = 0;
	Relay_Init();
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
                    OLED_ShowCH(32, 2, "验证失败");
                    delay_ms(1500);
                }
            }
            OLED_Clear();
            OLED_ShowCH(32, 2, "请按手指");
        }
        delay_ms(50);
    }


}

//删除指纹
void Del_FR(void)
{
    u8  ensure;
    u16 ID_NUM = 0;
    OLED_ShowCH(0, 0, "4加2减 3确认  ");
    OLED_ShowCH(0, 2, " K5清空指纹库 ");
    OLED_ShowCH(0, 4, "1返回 0<=ID<=99");
    while(1)
    {
        u8 key_now = Button4_4_Scan();
        if(key_now == 2) //减
        {
            if(ID_NUM > 0) ID_NUM--;
        }
        else if(key_now == 4) //加
        {
            if(ID_NUM < 99) ID_NUM++;
        }
        else if(key_now == 1) //相当于原先KEY1返回
        {
            goto MENU;
        }
        else if(key_now == 5) //清空指纹库
        {
            ensure = PS_Empty();
            if(ensure == 0)
            {
                OLED_Clear();
                OLED_ShowCH(0, 2, " 清空指纹库成功 ");
            }
            else ShowErrMessage(ensure);
            delay_ms(1500);
            goto MENU;
        }
        else if(key_now == 3) //确定删除指定ID
        {
            ensure = PS_DeletChar(ID_NUM, 1);
            if(ensure == 0)
            {
                OLED_Clear();
                OLED_ShowCH(0, 2, "  删除指纹成功  ");
            }
            else ShowErrMessage(ensure);
            delay_ms(1500);
            goto MENU;
        }
        //实时显示 ID
        OLED_ShowCH(40, 6, "ID=");
        OLED_ShowNum(65, 6, ID_NUM, 2, 1);
        delay_ms(100);
    }

MENU:
    OLED_Clear();
                OLED_ShowCH(16,0,"K14改密码");
                OLED_ShowCH(16,2,"K1添指纹");
                OLED_ShowCH(16,4,"K3删指纹");
                OLED_ShowCH(16,6,"K15退出并关锁");
}
