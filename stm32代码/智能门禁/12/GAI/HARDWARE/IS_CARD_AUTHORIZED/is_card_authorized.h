#ifndef __IS_CARD_AUTHORIZED_H
#define __IS_CARD_AUTHORIZED_H

#define MAX_CARDS 10

extern unsigned char card_list[MAX_CARDS][4];
extern unsigned char card_count;

// 现在不需要参数，直接void（因为它内部检测刷卡）
int is_authorized_card(void);

// 添加卡依然是手动传入刷到的uid
int add_new_card(unsigned char *uid);

void add_card_mode(void);

int delete_card(unsigned char *uid);
void delete_card_mode(void);


#endif
