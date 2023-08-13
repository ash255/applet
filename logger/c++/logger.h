#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <stdio.h>
#include <string>
#include <windows.h>
using std::string;

enum LOGGER_LEVEL
{
	LEVEL_DEBUG,
	LEVEL_MESSAGE,
	LEVEL_ERROR,
	LEVEL_LMAC,
	LEVEL_RLC,
	LEVEL_LAST
};

class logger
{
public:
	static logger *get_logger(FILE* fp);
	static logger *get_logger(string file);
	static void set_level(LOGGER_LEVEL level) { logger::m_level[level] = true; }
	static void clear_level(LOGGER_LEVEL level) { logger::m_level[level] = false; }
	static bool check_level(LOGGER_LEVEL level) { return logger::m_level[level]; }
	static void printf_line(LOGGER_LEVEL level, const char *file, const char *func, char *format, ...);
	static void printf_color_line(LOGGER_LEVEL level, int color, const char *file, const char *func, char *format, ...);
	static void printf_bytes(LOGGER_LEVEL level, uint8_t *buffer, int len, const char *file, const char *func, char *format = nullptr, ...);
	static void printf_bits(LOGGER_LEVEL level, uint8_t *buffer, int len, const char *file, const char *func, char *format = nullptr, ...);
	static void printf_raw(LOGGER_LEVEL level, char *format = nullptr, ...);
private:
	logger(): m_fp(stdout) { }
	static char *file_name(char *path);
	~logger();
	FILE *m_fp;
	static logger *m_logger;
	static int m_cur_color;
	static bool m_level[LEVEL_LAST];
};

#define __DEBUG(fmt, ...) logger::printf_color_line(LEVEL_DEBUG, FOREGROUND_GREEN | FOREGROUND_INTENSITY, __FILE__, __func__, fmt, ##__VA_ARGS__)
#define __MESSAGE(fmt, ...) logger::printf_color_line(LEVEL_MESSAGE, FOREGROUND_BLUE | FOREGROUND_INTENSITY, __FILE__, __func__, fmt, ##__VA_ARGS__)
#define __ERROR(fmt, ...) logger::printf_color_line(LEVEL_ERROR, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_RED, __FILE__, __func__, fmt, ##__VA_ARGS__)
#define __LMAC(fmt, ...) logger::printf_color_line(LEVEL_LMAC, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, __FILE__, __func__, fmt, ##__VA_ARGS__)
#define __RLC(fmt, ...) logger::printf_color_line(LEVEL_RLC, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY, __FILE__, __func__, fmt, ##__VA_ARGS__)

#endif
