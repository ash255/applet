#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <stdio.h>

#define __LEVEL_DEBUG__ 0
#define __LEVEL_MESSAGE__ 1
#define __LEVEL_ERROR__ 2
#define __LEVEL_LAST__ 3

void log_printf(const char *level, const char *module, const char *format, ...);
void log_with_fd(FILE *fp);
void log_with_file(char *file);
void log_set_level(int level);
void log_clear_level(int level);
int chekc_level(int level);
void log_printf_array(const char *level, const char *module, char *buffer, int len);

#define __LOG_DEBUG__(fmt, ...)                      \
    {                                                     \
        if (chekc_level(__LEVEL_DEBUG__))                 \
        {                                                 \
            log_printf("DEBUG", __func__, fmt, ##__VA_ARGS__); \
        }                                                 \
    }
#define __LOG_MESSAGE__(fmt, ...)                      \
    {                                                       \
        if (chekc_level(__LEVEL_MESSAGE__))                 \
        {                                                   \
            log_printf("MESSAGE", __func__, fmt, ##__VA_ARGS__); \
        }                                                   \
    }
#define __LOG_ERROR__(fmt, ...)                      \
    {                                                     \
        if (chekc_level(__LEVEL_ERROR__))                 \
        {                                                 \
            log_printf("ERROR", __func__, fmt, ##__VA_ARGS__); \
        }                                                 \
    }

#define __LOG_DEBUG_ARRAY__(buffer, len)            \
    {                                                    \
        if (chekc_level(__LEVEL_DEBUG__))                \
        {                                                \
            log_printf_array("DEBUG", __func__, buffer, len); \
        }                                                \
    }

#define __LOG_MESSAGE_ARRAY__(buffer, len)            \
    {                                                      \
        if (chekc_level(__LEVEL_MESSAGE__))                \
        {                                                  \
            log_printf_array("MESSAGE", __func__, buffer, len); \
        }                                                  \
    }

#define __LOG_ERROR_ARRAY__(buffer, len)            \
    {                                                    \
        if (chekc_level(__LEVEL_ERROR__))                \
        {                                                \
            log_printf_array("ERROR", __func__, buffer, len); \
        }                                                \
    }

#endif