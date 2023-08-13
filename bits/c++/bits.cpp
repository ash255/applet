#include "bits.h"
#include <stdio.h>
#include "logger.h"

#ifndef __ERROR
#define __ERROR printf
#endif

int st_bits::memmem(ubit_t *src, int src_len, ubit_t *search, int search_len)
{
	ubit_t *pSrc = src;
	ubit_t *pEnd = src + src_len - search_len;
	while (pSrc <= pEnd)
	{
		if (memcmp(pSrc, search, search_len) == 0)
			return (int)(pSrc - src);
		pSrc++;
	}
	return 0;
}

st_bits::st_bits(uint8_t * data, int len, int bits_mode, BITS_ENDIAN endian) :m_bits(nullptr), m_endian(endian)
{
	if (bits_mode == 1)
	{
		this->m_bits = new ubit_t[len];
		this->m_len = len;
		for (int i = 0; i < this->m_len; i++)
		{
			this->m_bits[i] = data[i] & 1;
		}
	}
	else
	{
		//this->m_bits = st_bits::hex_to_bits(data, len, endian);
		this->m_bits = st_bits::hex_to_bits(data, len, LEFT_BIG_ENDIAN);
		this->m_len = len * 8;
	}
}

st_bits::st_bits(string str, int bits_mode, BITS_ENDIAN endian) :m_bits(nullptr), m_endian(endian)
{
	if (bits_mode == 1)
	{
		this->m_bits = new ubit_t[str.length()];
		this->m_len = (int)str.length();
		for (int i = 0; i < this->m_len; i++)
		{
			this->m_bits[i] = (str[i] - '0') & 1;
		}
	}
	else
	{
		if ((str.length() % 2) != 0)
		{
			if (endian == LEFT_BIG_ENDIAN || endian == LEFT_LITTLE_ENDIAN)
				str += '0';
			else
				str = '0' + str;
		}

		int bytes_len = (int)str.length() / 2;
		uint8_t *bytes = new uint8_t[bytes_len];
		char buf[3] = { 0 };
		for (int i = 0; i < bytes_len; i++)
		{
			buf[0] = str[i * 2];
			buf[1] = str[i * 2 + 1];
			bytes[i] = (uint8_t)strtol(buf, NULL, 16);
		}

		this->m_bits = st_bits::hex_to_bits(bytes, bytes_len, endian);
		this->m_bits = st_bits::hex_to_bits(bytes, bytes_len, LEFT_BIG_ENDIAN);
		this->m_len = bytes_len * 8;
		delete[](bytes);
	}
}

st_bits::~st_bits()
{
	if (this->m_bits != nullptr)
		delete[](this->m_bits);
}

st_bits* st_bits::bits_form_hex_file(string hex_file, BITS_ENDIAN endian)
{
	uint8_t *hex_data;
	int hex_data_len;
	int ret = st_bits::read_file(hex_file, &hex_data, &hex_data_len);
	if (ret == 0 || hex_data_len == 0)
	{
		__ERROR("bits_form_hex_file(%s, %d) failed\n", hex_file.c_str(), endian);
		return nullptr;
	}
	
	st_bits *ret_bits = new st_bits(hex_data, hex_data_len, 0,  endian);
	delete[](hex_data);
	return ret_bits;
}

st_bits* st_bits::bits_form_bin_file(string bin_file, BITS_ENDIAN endian)
{
	ubit_t *bin_data;
	int hin_data_len;
	int ret = st_bits::read_file(bin_file, (uint8_t**)&bin_data, &hin_data_len);
	if (ret == 0)
	{
		__ERROR("bits_form_bin_file(%s, %d) failed\n", bin_file.c_str(), endian);
		return nullptr;
	}
	st_bits *ret_bits = new st_bits(bin_data, hin_data_len, 1, endian);
	delete[](bin_data);
	return ret_bits;
}

ubit_t * st_bits::hex_to_bits(uint8_t * hex_data, int len, BITS_ENDIAN endian)
{
	int bits_len = len * 8;
	ubit_t *st_bits = new ubit_t[bits_len];
	ubit_t *bits_copy = st_bits;
	if (endian == LEFT_BIG_ENDIAN || endian == RIGHT_BIG_ENDIAN)
	{
		for (int i = 0; i < len; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				*st_bits++ = (hex_data[i] >> (7 - j)) & 1;
			}
		}
	}
	if (endian == LEFT_LITTLE_ENDIAN || endian == RIGHT_LITTLE_ENDIAN)
	{
		for (int i = 0; i < len; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				*st_bits++ = (hex_data[i] >> j) & 1;
			}
		}
	}
	return bits_copy;
}

uint8_t * st_bits::bits_to_hex(ubit_t * bin_data, int len, BITS_ENDIAN endian)
{
	int hex_len = (len + 7) / 8;
	uint8_t *hex = new uint8_t[hex_len];
	uint8_t *hex_copy = hex;
	uint8_t offset = 0, tmp = 0;
	if (endian == LEFT_BIG_ENDIAN)
	{
		for (int i = 0; i < len; i++)
		{
			tmp |= bin_data[i] << (7 - offset++);
			if (offset == 8)
			{
				*hex++ = tmp;
				offset = tmp = 0;
			}
		}
		if (len % 8 != 0)
		{
			*hex++ = tmp;
		}
	}
	else if (endian == LEFT_LITTLE_ENDIAN)
	{
		for (int i = 0; i < len; i++)
		{
			tmp |= bin_data[i] << offset++;
			if (offset == 8)
			{
				*hex++ = tmp;
				offset = tmp = 0;
			}
		}
		if (len % 8 != 0)
		{
			*hex++ = tmp << (8 - len % 8);
		}
	}
	else if (endian == RIGHT_BIG_ENDIAN)
	{
		hex += hex_len - 1;
		for (int i = len-1; i >=0; i--)
		{
			tmp |= bin_data[i] << offset++;
			if (offset == 8)
			{
				*hex-- = tmp;
				offset = tmp = 0;
			}
		}
		if (len % 8 != 0)
		{
			*hex-- = tmp << (8 - len % 8);
		}
	}
	else if (endian == RIGHT_LITTLE_ENDIAN)
	{
		hex += hex_len - 1;
		for (int i = len - 1; i >= 0; i--)
		{
			tmp |= bin_data[i] << (7 - offset++);
			if (offset == 8)
			{
				*hex-- = tmp;
				offset = tmp = 0;
			}
		}
		if (len % 8 != 0)
		{
			*hex-- = tmp;
		}
	}

	return hex_copy;
}

void st_bits::bits_reverse()
{
	for (int i = 0; i < this->m_len; i++)
	{
		this->m_bits[i] ^= 1;
	}
}

int st_bits::search(string bits, int start)
{
	if (start > this->m_len)
	{
		return 0;
	}
	ubit_t *b = new ubit_t[bits.length()];

	for (int i = 0; i < bits.length(); i++)
	{
		b[i] = bits[i] - '0';
	}

	int ret = memmem(&this->m_bits[start], this->m_len - start, b, (int)bits.length());
	delete[](b);
	return ret;
}

st_bits* st_bits::truncate(int start, int len)
{
	if (start < 0)
	{
		start = 0;
	}
	if ((start + len) > this->m_len)
	{
		len = this->m_len - start;
	}
	return new st_bits(&this->m_bits[start], len, 1, this->m_endian);
}

string st_bits::hex_str()
{
	int hex_len = (this->m_len + 7) / 8;
	uint8_t *hex = bits_to_hex(this->m_bits, this->m_len, this->m_endian);

	int hex_str_len = hex_len * 2 + 1;
	char *hex_str = new char[hex_str_len];
	char *hex_str_copy = hex_str;

	for (int i = 0; i < hex_len; i++)
	{
		sprintf(hex_str, "%02X", hex[i]);
		hex_str += 2;
	}

	hex_str_copy[hex_str_len - 1] = 0;
	string ret_hex = string(hex_str_copy);
	delete[](hex_str_copy);
	return ret_hex;
}

string st_bits::bin_str(int start)
{
	if (start >= this->m_len)
	{
		start = this->m_len;
	}
	int bin_str_len = this->m_len - start + 1;
	if (bin_str_len > 0)
	{
		char *bin_str = new char[bin_str_len];

		for (int i = start; i < this->m_len; i++)
		{
			bin_str[i - start] = '0' + (this->m_bits[i] & 1);
		}

		bin_str[bin_str_len - 1] = 0;
		string ret_hex = string(bin_str);
		delete[](bin_str);
		return ret_hex;
	}
	return "";
}

int st_bits::read_file(string file, uint8_t ** data, int *len)
{
	FILE *fp = nullptr;
	fopen_s(&fp, file.c_str(), "rb");
	if (fp != nullptr)
	{
		fseek(fp, 0, SEEK_END);
		long file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (file_size > 0)
		{
			*data = new uint8_t[file_size];
			*len = (int)file_size;
			int rlen = (int)fread(*data, 1, file_size, fp);
			return 1;
		}
		fclose(fp);
	}
	return 0;
}

int st_frame::unpack_i(int start, int len)
{
	if (len > (sizeof(int) * 8))
	{
		__ERROR("len %d too long\n", len);
		return 0;
	}
	int used_len = 0;
	int ret = this->m_bits->unpack_i(start, len, &used_len);
	if (used_len != len)
	{
		__ERROR("bits buffer exhaust, expect len=%d  get len=%d\n", len, used_len);
		return 0;
	}
	this->m_start = start + used_len;
	return ret;
}

double st_frame::unpack_d(int start)
{
	int used_len = 0;
	double ret = this->m_bits->unpack_d(start, &used_len);
	if (used_len != (sizeof(double) * 8))
	{
		__ERROR("bits buffer exhaust, expect len=%d  get len=%d\n", 64, used_len);
		return 0;
	}
	this->m_start = start + used_len;
	return ret;
}

float st_frame::unpack_f(int start)
{
	int used_len = 0;
	float ret = this->m_bits->unpack_f(start, &used_len);
	if (used_len != (sizeof(float) * 8))
	{
		__ERROR("bits buffer exhaust, expect len=%d  get len=%d\n", 32, used_len);
		return 0;
	}
	this->m_start = start + used_len;
	return ret;
}

void st_frame::set_start(int start)
{
	if (start > this->m_bits->get_len())
	{
		__ERROR("start > bits_len, start=%d bits_len=%d\n", start, this->m_bits->get_len());
	}
	else
	{
		this->m_start = start;
	}
}

#if 0
void test1()
{
	BITS_ENDIAN endian_mode[] = { LEFT_BIG_ENDIAN, LEFT_LITTLE_ENDIAN, RIGHT_BIG_ENDIAN, RIGHT_LITTLE_ENDIAN };

	for (int i = 0; i < 4; i++)
	{
		bits *test = new bits("00110011110110101", 1, endian_mode[i]);
		printf("%s\n", test->hex_str().c_str());
		delete(test);
	}
}
int main()
{
	test1();
}

#endif
