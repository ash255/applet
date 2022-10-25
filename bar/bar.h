#ifndef __BAR_H
#define __BAR_H

/*
                       | optional  |
    [tag][>>******][2%](2/100)(2.0s) 
*/
#ifndef __cplusplus
typedef char bool;
#define true (1)
#define false (0)
#endif
typedef struct bar_info_t
{
    char *tag;          //���� 
    int  bar_len;       //���������ȣ�������[] 
    char used_ch;       //Ĭ��Ϊ>
    char unused_ch;     //Ĭ��λ* 
    bool number_flag;   //�Ƿ���ʾ(2/100)
    bool time_flag;     ////�Ƿ���ʾ(2.0s)
    
    unsigned long cur;
    unsigned long max;
    unsigned long start_time;
}bar_info_t;

void bar_reset(bar_info_t *bar);
bar_info_t* bar_default();
void bar_set(bar_info_t* bar, const char*tag, int max);
void bar_show(bar_info_t *bar);
void bar_add(bar_info_t *bar);

#endif
