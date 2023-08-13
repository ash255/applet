#include "logger.h"
#include <stdarg.h>
#include <windows.h>

#define default_color (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY)
logger *logger::m_logger = nullptr;
int logger::m_cur_color = default_color;
bool logger::m_level[LEVEL_LAST] = { 0 };

logger * logger::get_logger(FILE * fp)
{
	if (logger::m_logger == nullptr)
	{
		logger::m_logger = new logger();
		logger::m_logger->m_fp = fp;
	}
	return logger::m_logger;
}

logger * logger::get_logger(string file)
{
	if (logger::m_logger == nullptr)
	{
		FILE *fp = nullptr;
		fopen_s(&fp, file.c_str(), "w");
		if (fp != nullptr)
		{
			logger::m_logger = new logger();
			logger::m_logger->m_fp = fp;
		}
		else
		{
			printf("can't create logger file at %s\n", file.c_str());
			return nullptr;
		}
	}
	return logger::m_logger;
}

void logger::printf_line(LOGGER_LEVEL level, const char *file, const char *func, char *format, ...)
{
	if (logger::m_logger != nullptr && logger::check_level(level))
	{
		va_list va;
		va_start(va, format);
		fprintf(logger::m_logger->m_fp, "[%s:%s] ", logger::file_name((char*)file), func);
		vfprintf(logger::m_logger->m_fp, format, va);
		fflush(logger::m_logger->m_fp);
		va_end(va);
	}
}

void logger::printf_color_line(LOGGER_LEVEL level, int color, const char *file, const char *func, char * format, ...)
{
	if (logger::m_logger != nullptr && logger::check_level(level))
	{
		logger::m_cur_color = color;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
		va_list va;
		va_start(va, format);
		fprintf(logger::m_logger->m_fp, "[%s:%s] ", logger::file_name((char*)file), func);
		vfprintf(logger::m_logger->m_fp, format, va);
		fflush(logger::m_logger->m_fp);
		va_end(va);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), default_color);
	}
}

void logger::printf_bytes(LOGGER_LEVEL level, uint8_t * buffer, int len, const char *file, const char *func, char *format, ...)
{
	if (logger::m_logger != nullptr && logger::check_level(level))
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), logger::m_cur_color);
		va_list va;
		va_start(va, format);
		fprintf(logger::m_logger->m_fp, "[%s:%s] ", logger::file_name((char*)file), func);
		if (format != nullptr)
		{
			vfprintf(logger::m_logger->m_fp, format, va);
		}
		fprintf(logger::m_logger->m_fp, "<%d> - ", len);
		for (int i = 0; i < len; i++)
		{
			fprintf(logger::m_logger->m_fp, "%02X ", buffer[i]);
		}
		fflush(logger::m_logger->m_fp);
		va_end(va);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), default_color);
	}
}

void logger::printf_bits(LOGGER_LEVEL level, uint8_t * buffer, int len, const char *file, const char *func, char *format, ...)
{
	if (logger::m_logger != nullptr && logger::check_level(level))
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), logger::m_cur_color);
		va_list va;
		va_start(va, format);
		fprintf(logger::m_logger->m_fp, "[%s:%s] ", logger::file_name((char*)file), func);
		if (format != nullptr)
		{
			vfprintf(logger::m_logger->m_fp, format, va);
		}
		fprintf(logger::m_logger->m_fp, "<%d> - ", len);
		for (int i = 0; i < len; i++)
		{
			fprintf(logger::m_logger->m_fp, "%d", buffer[i]);
		}
		fflush(logger::m_logger->m_fp);
		va_end(va);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), default_color);
	}
}

void logger::printf_raw(LOGGER_LEVEL level, char * format, ...)
{
	if (logger::m_logger != nullptr && logger::check_level(level))
	{
		va_list va;
		va_start(va, format);
		vfprintf(logger::m_logger->m_fp, format, va);
		fflush(logger::m_logger->m_fp);
		va_end(va);
	}
}

char * logger::file_name(char * path)
{
	char *pos = strrchr(path, '\\');
	return pos == nullptr ? path : pos + 1;
}

logger::~logger()
{
	if (logger::m_logger != nullptr)
	{
		fflush(logger::m_logger->m_fp);
		fclose(logger::m_logger->m_fp);
		delete(logger::m_logger);
		logger::m_logger = nullptr;
	}
}
