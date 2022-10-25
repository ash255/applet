#define _XOPEN_SOURCE /* See feature_test_macros(7) */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stddef.h>
#include <errno.h>
#include <sys/time.h>
#include "logger.h"

#define __IN
#define __OUT
#define NUMBER "0123456789"
#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER "abcdefghijklmnopqrstuvwxyz"
#define DES_WORD LOWER LOWER NUMBER "\\."
#define PWD_BUF_SIZE (1024*1024)
#define PWD_BUF_NUM  (8)
#define THREAD_NUM (5)

const static char* g_crypt_type_string[7] = 
{
    "des56",
    "md5",
    "blowfish",
    "",
    "",
    "sha-256",
    "sha-512"
};

struct thread_info_t
{
    int tid;
    char *salt;
    char *expect_pwd;
};

struct slot_t
{
    char *cand;
    int cand_len;
    int cur_idx;
    struct slot_t *prev;
};

struct pwd_buf_t
{
    int status; //0=free 1=ready 2=used
    int len;
    char pwd_buf[1];
};

static pthread_mutex_t g_pwd_buf_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_pwd_buf_ready = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_pwd_buf_empty = PTHREAD_COND_INITIALIZER;
//static todo: 事件量，用于通知工作线程password buffer已经准备好了
static struct pwd_buf_t *g_pwd_buf[PWD_BUF_NUM] = {0};
static int g_thread_ctl = 0;    //0: 正常运行  1：找到密钥  2：字典已空

static int get_pwd_from_slot(__OUT char *pwd, __IN struct slot_t* pwd_slot, __IN int slot_len)
{
    int i;
    struct slot_t *slot = pwd_slot;
    char *cur_pwd_ptr = pwd;
    
    for(i=0;i<slot_len;i++)
    {
        *cur_pwd_ptr++ = slot->cand[slot->cur_idx];
        slot = slot->prev;
    }
    *cur_pwd_ptr = 0;
    // static int debug_cnt = 50;
    // if(debug_cnt-- > 0)
        // printf("pwd: %s\n", pwd);
    
    slot = pwd_slot;
    while(slot != NULL)
    {
        slot->cur_idx++;
        if(slot->cur_idx == slot->cand_len)
        {
            slot->cur_idx = 0;
            if(slot->prev == NULL)
            {
                break;
            }
            slot = slot->prev;
            continue;
        }
        break;
    }
    
    if(slot != NULL && slot->cand_len == 0 && slot->cur_idx > 0) //遍历结束的条件
    {
        return 0;
    }
    return 1;
}

static int init_pwd_buf()
{
    int i;
    
    //初始化pwd_buf
    for(i=0; i<PWD_BUF_NUM; i++)
    {
        g_pwd_buf[i] = (struct pwd_buf_t*)malloc(PWD_BUF_SIZE);
        if(g_pwd_buf[i] == NULL)
        {
            __LOG_ERROR__("can't not alloc password buffer, id=%d\n", i);
            goto __init_pwd_buf_end;
        }
        g_pwd_buf[i]->status = 0;
        g_pwd_buf[i]->len = PWD_BUF_SIZE - offsetof(struct pwd_buf_t, pwd_buf);
    }
    return 1;
__init_pwd_buf_end:
    for(i=0; i<PWD_BUF_NUM; i++)
    {
        if(g_pwd_buf[i] != NULL)
        {
            free(g_pwd_buf[i]);
        }
    }
    return 0;
}

static int passwd_tasklet(__IN struct slot_t *pwd_slot, __IN int slot_len)
{
    int i, j;
    char *cur_buf_ptr = NULL;
    int left_size = PWD_BUF_SIZE;
    unsigned int pwd_cnt = 0;
    
    __LOG_DEBUG__("passwd_tasklet start\n");
    while(1)
    {
        if(g_thread_ctl == 1 || g_thread_ctl == 2)
            break;
        pthread_mutex_lock(&g_pwd_buf_mutex);
        pthread_cond_wait(&g_pwd_buf_empty, &g_pwd_buf_mutex);
        for(i=0; i<PWD_BUF_NUM; i++)
        {
            cur_buf_ptr = g_pwd_buf[i]->pwd_buf;
            if(g_pwd_buf[i]->status == 0)    //0=free 1=ready 2=used
            {
                //do your work
                for(j=0; j < ((g_pwd_buf[i]->len - 1) / (slot_len + 1)); j++) //-1: 至少留一个字节给最后的null, +1:算上每个字符串的/0
                {
                    if(get_pwd_from_slot(cur_buf_ptr, pwd_slot, slot_len) == 0)
                    {
                        g_thread_ctl = 2;
                        break;
                    } 
                    cur_buf_ptr += strlen(cur_buf_ptr) + 1;
                    pwd_cnt++;
                }
                *cur_buf_ptr = 0;
                g_pwd_buf[i]->status = 1;
                if(g_thread_ctl == 1 || g_thread_ctl == 2)
                    break;
            }
        }
        __LOG_DEBUG__("pwd product: %d\n", pwd_cnt);
        pthread_cond_signal(&g_pwd_buf_ready);
        pthread_mutex_unlock(&g_pwd_buf_mutex);
    }
    pthread_cond_signal(&g_pwd_buf_ready);
    __LOG_DEBUG__("passwd_tasklet end\n");
    return 1;
}

static void *passwd_thread(__IN void *arg)
{
    int i;
    struct pwd_buf_t *cur_pwd_buf = NULL;
    struct thread_info_t *tinfo = (struct thread_info_t *)arg;
    char *cur_buf_ptr = NULL;
    struct timespec time = {0};
    struct timeval now = {0};
    
    __LOG_DEBUG__("thread %d start\n", tinfo->tid);
    while(1)
    {
        if(g_thread_ctl == 1)
        {
            break;
        }
        pthread_mutex_lock(&g_pwd_buf_mutex);
        cur_pwd_buf = NULL;
        for(i=0; i<PWD_BUF_NUM; i++)
        {
            if(g_pwd_buf[i]->status == 1)    //0=free 1=ready 2=used
            {
                g_pwd_buf[i]->status = 2;
                cur_pwd_buf = g_pwd_buf[i];
                break;
            }
        }
        if(cur_pwd_buf == NULL)
        {
            if(g_thread_ctl == 2)   //字典已空，且无pwd_buf
            {
                pthread_mutex_unlock(&g_pwd_buf_mutex);
                break;
            }
            do{
                pthread_cond_signal(&g_pwd_buf_empty);
                gettimeofday(&now, NULL);
                time.tv_sec = now.tv_sec + 1;
                time.tv_nsec = now.tv_usec * 1000;
            }while(pthread_cond_timedwait(&g_pwd_buf_ready, &g_pwd_buf_mutex, &time) == ETIMEDOUT);
            pthread_mutex_unlock(&g_pwd_buf_mutex);
            continue;
        }
        pthread_mutex_unlock(&g_pwd_buf_mutex);
        //do your work
        cur_buf_ptr = cur_pwd_buf->pwd_buf;
        while(1)
        {
            if(cur_buf_ptr[0] == '\0')
            {
                break;
            }
            if(strcmp(crypt(cur_buf_ptr, tinfo->salt), tinfo->expect_pwd) == 0)
            {
                __LOG_MESSAGE__("password found: %s <=> %s\n", tinfo->expect_pwd, cur_buf_ptr);
                g_thread_ctl = 1;
                break;
            }
            cur_buf_ptr = cur_buf_ptr + strlen(cur_buf_ptr) + 1;
        }
        cur_pwd_buf->status = 0;
    }
    pthread_cond_signal(&g_pwd_buf_ready);
    __LOG_DEBUG__("thread %d end\n", tinfo->tid);
    return NULL;
}

static void print_help()
{
    __LOG_MESSAGE__("\n"
                    "-h: print this helf\n"
                    "-f: input from file, such as /etc/shadow\n"
                    "-i: input from string, such as $1$AXP3QWMEMLZXC92QWL11\n"
                    "-d: dict file, split by \\n\n"
                    "-t: thread number, defalut 5\n"
                    "example: password:123456  encrypt:%s\n", crypt("123456","ab"));
}

/*
    return crypt type
    -1: unknown
    0: linux defalut des56
    1: md5
    2: blowfish
    5: sha-256
    6: sha-512
*/
static int get_salt(__IN char *passwd, __OUT char *slat, __IN int slat_len)
{
    int pwd_len = strlen(passwd);
    int crypt_type = -1;
    
    if(pwd_len != 13 && passwd[0] == '$')
    //if(pwd_len > 3 && passwd[0] == '$' && passwd[2] == '$')
    {
        strncpy(slat, &passwd[3], slat_len - 1);
        crypt_type = passwd[1] - '0';
    }else
    {
        slat[0] = passwd[0];
        slat[1] = passwd[1];
        crypt_type = 0;
    }
    return crypt_type;
}

static int make_pwd_slot(__OUT struct slot_t *slot, __IN int len, __IN char *cand)
{
    int i;
    struct slot_t *prev, *cur = slot;
    
    for(i=0;i<len;i++)
    {
        cur->cand = cand;
        cur->cand_len = strlen(cand);
        cur->cur_idx = 0;
        cur->prev = (struct slot_t *)malloc(sizeof(struct slot_t));
        __LOG_DEBUG__("cand_len: %d cand: %s\n", cur->cand_len, cur->cand);
        cur = cur->prev;
    }
    //设置结束的slot
    cur->prev = NULL;
    cur->cand_len = 0;
    cur->cur_idx = 0;
    return 1;
}

static int start_attack(__IN int td_num, __IN char *passwd, __IN char *salt)
{
    int i;
    pthread_t *thd = NULL;
    struct thread_info_t *td_info = NULL;
    struct slot_t pwd_slot;
    
    if(td_num <= 0)
    {
        __LOG_ERROR__("thread num error, %d\n", td_num);
        return 0;
    }
    if(init_pwd_buf() == 0)
    {
        __LOG_ERROR__("init password buffer error\n");
        return 0;
    }
    
    thd = (pthread_t *)malloc(td_num * sizeof(pthread_t));
    td_info = (struct thread_info_t*)malloc(td_num * sizeof(struct thread_info_t));
    for(i=0; i<td_num; i++)
    {
        td_info[i].tid = i;
        td_info[i].salt = salt;
        td_info[i].expect_pwd = passwd;
        if(pthread_create(&thd[i], NULL, passwd_thread, &td_info[i]) != 0)
        {
            __LOG_ERROR__("can not create thread, num=%d/%d\n", i, td_num);
            break;
        }
    }
    
    // make_pwd_slot(&pwd_slot, 6, NUMBER"\\.");
    make_pwd_slot(&pwd_slot, 6, "123456");
    passwd_tasklet(&pwd_slot, 6);
    
    for(i=0; i<td_num; i++)
    {
        pthread_join(thd[i], NULL);
    }
    
    struct slot_t *tmp = NULL;
    while(pwd_slot.prev != NULL)
    {
        tmp = pwd_slot.prev;
        pwd_slot.prev = pwd_slot.prev->prev;
        free(tmp);
    }
    
    if(thd != NULL)
    {
        free(thd);
    }
    
    return 1;
}

int main(int argc, char **argv)
{
    int type, i, td_num = THREAD_NUM;
    char salt[64] = {0};
    char passwd[128] = {0};
    
    log_with_fd(stdout);
    log_set_level(__LEVEL_DEBUG__);
    log_set_level(__LEVEL_MESSAGE__);
    log_set_level(__LEVEL_ERROR__);
    
    if(argc < 3)
    {
        print_help();
        return -1;
    }
    
    
    for(i=1;i<argc;i++)
    {
        if(strcmp(argv[i], "-h") == 0)
        {
            print_help();
            break;
        }else if(strcmp(argv[i], "-t") == 0)
        {
            if(++i >= argc)
            {
                print_help();
                return -1;
            }
            td_num = strtoul(argv[i], NULL, 10);
        }
        else if(strcmp(argv[i], "-d") == 0)
        {
            if(++i >= argc)
            {
                print_help();
                return -1;
            }
            __LOG_ERROR__("dict no implement\n");
        }
        else if(strcmp(argv[i], "-f") == 0)
        {
            if(++i >= argc)
            {
                print_help();
                return -1;
            }
            __LOG_ERROR__("file no implement\n");
        }else if(strcmp(argv[i], "-i") == 0)
        {
            if(++i >= argc)
            {
                print_help();
                return -1;
            }
            type = get_salt(argv[i], salt, sizeof(salt));
            if(type == -1)
            {
                __LOG_ERROR__("get slat failed\n");
                return -1;
            }
            strncpy(passwd, argv[i], sizeof(passwd) - 1);
            __LOG_MESSAGE__("crypt type: %d %s\n", type, g_crypt_type_string[type]);
        }else
        {
            __LOG_ERROR__("invalid parameter\b");
        }
    }
    __LOG_MESSAGE__("password: %s\n", passwd);
    __LOG_MESSAGE__("salt: %s\n", salt);
    
    if(strlen(passwd) == 0 || strlen(salt) == 0)
    {
        print_help();
    }else
    {
        start_attack(td_num, passwd, salt);
    }
    
    return 0;
}