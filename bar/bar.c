#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "bar.h"

static bar_info_t g_bar_default = { NULL, 40, '>', '*', true, true, 0, 0, 0 };

void bar_reset(bar_info_t *bar)
{
    if(bar)
    {
        bar->start_time = bar->max = bar->cur = 0;
    }
}

bar_info_t* bar_default()
{
    bar_reset(&g_bar_default);
    return &g_bar_default;
}

void bar_set(bar_info_t* bar, const char*tag, int max)
{
    if(bar)
    {
        bar_reset(bar);
        bar->tag = (char*)tag;
        bar->max = max;
    }
}

void bar_show(bar_info_t *bar)
{
    int i;
    
    if(bar)
    {
        float percent = (float)bar->cur / (float)bar->max * 100.0f;
        int used_num = percent / 100.0f * bar->bar_len;
        int unused_num = bar->bar_len - used_num;
        
        
        printf("\r");
        if(bar->tag)
        {
            printf("[%s]", bar->tag);
        }
        
        printf("[");
        for(i=0;i<used_num;i++)
        {
            printf("%c", bar->used_ch);
        }
        for(i=0;i<unused_num;i++)
        {
            printf("%c", bar->unused_ch);
        }
        printf("]");
        
        printf("[%.2f%%]", percent);
        
        if(bar->number_flag)
        {
            printf("(%d/%d)", bar->cur, bar->max);
        }
        
        if(bar->time_flag)
        {
            if(bar->start_time == 0)
            {
                printf("(0.0s)");
                //todo: only work on windows
                bar->start_time = GetTickCount();
            }  
            else
            {
                //todo: only work on windows
                unsigned long cur_tick = GetTickCount();
                float start_time = (cur_tick - bar->start_time)/1000.0f;
                printf("(%.1fs)", start_time);    
            }
        }
    }  
}

void bar_add(bar_info_t *bar)
{
    if(bar)
    {
        if(bar->cur < bar->max)
            bar->cur++;
    }
}

#if 0
int main(int argc, char *argv[])
{ 
    int i;
    
    bar_info_t* bar = bar_default();
    bar_set(bar, "test", 200);
    
    for(i=0;i<100;i++)
    {
        bar_add(bar);
        bar_show(bar);
        usleep(50000);
    } 
    return 0;
}
#endif
