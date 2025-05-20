#ifndef __IS_CARD_AUTHORIZED_H
#define __IS_CARD_AUTHORIZED_H

#define MAX_CARDS 10

extern unsigned char card_list[MAX_CARDS][4];
extern unsigned char card_count;

// ���ڲ���Ҫ������ֱ��void����Ϊ���ڲ����ˢ����
int is_authorized_card(void);

// ��ӿ���Ȼ���ֶ�����ˢ����uid
int add_new_card(unsigned char *uid);

void add_card_mode(void);

int delete_card(unsigned char *uid);
void delete_card_mode(void);


#endif
