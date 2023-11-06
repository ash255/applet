#include "bits.h"
#include <stdio.h>

int cbits::memmem(ubit_t *src, int src_len, ubit_t *search, int search_len)
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

cbits::cbits(uint8_t * data, int len, bool bits_mode, READ_MODE endian) :
	m_len_bits(0), m_len_bytes(0), m_data(nullptr), m_endian(endian)
{
	if (bits_mode)
	{
		this->m_data = cbits::bits_to_hex(data, len, LEFT_BIG_ENDIAN);
		this->m_len_bytes = (len + 7) / 8;
		this->m_len_bits = len;
	}
	else
	{
		this->m_data = new uint8_t[len];
		memcpy(this->m_data, data, len);
		this->m_len_bytes = len;
		this->m_len_bits = len * 8;
	}
}

cbits::cbits(string str, bool bits_mode, READ_MODE endian) :m_data(nullptr), m_endian(endian)
{
	if (bits_mode)
	{
		this->m_len_bytes = (str.length() + 7) / 8;
		while ((str.length() % 8) != 0)
		{
			if (endian == LEFT_BIG_ENDIAN || endian == LEFT_LITTLE_ENDIAN)
				str += '0';
			else
				str = '0' + str;
		}

		this->m_data = new uint8_t[this->m_len_bytes];
		this->m_len_bits = str.length();
		char buf[9] = { 0 };
		const char* pStr = str.data();
		for (int i = 0; i < this->m_len_bytes; i++)
		{
			memcpy(buf, pStr + i * 8, 8);
			this->m_data[i] = (uint8_t)strtol(buf, NULL, 2);
		}
	}
	else
	{
		while ((str.length() % 2) != 0)
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

		this->m_data = bytes;
		this->m_len_bytes = bytes_len;
		this->m_len_bits = bytes_len * 8;
	}
}

bool cbits::append(cbits* append_bits)
{
	if (append_bits->get_len_bytes() > 0)
	{
		int new_len_bytes = (this->m_len_bits + append_bits->get_len_bits() + 7) / 8;
		uint8_t* new_data = new uint8_t[new_len_bytes];
		if (new_data != nullptr)
		{
			if (this->m_len_bits != this->m_len_bytes * 8)
			{
				//比特不对齐，则需要把append的m_data移位进this->m_data
				uint8_t* pData = append_bits->get_data();
				int move_bits = this->m_len_bytes * 8 - this->m_len_bits;
				uint8_t mask = (1 << move_bits) - 1;
				this->m_data[this->m_len_bytes - 1] |= (pData[0] >> (8 - move_bits)) & mask;
				memcpy(new_data, this->m_data, this->m_len_bytes);
				int i;
				for (i = 0; i < append_bits->get_len_bytes() - 1; i++)
				{
					new_data[this->m_len_bytes + i] = (pData[0] << move_bits) | ((pData[1] >> (8 - move_bits)) & mask);
					pData++;
				}
				if ((append_bits->get_len_bits() - append_bits->get_len_bytes() * 8 - 8) > move_bits)
				{
					new_data[this->m_len_bytes + i] = pData[0] << move_bits;
				}
			}
			else
			{
				memcpy(new_data, this->m_data, this->m_len_bytes);
				memcpy(new_data, this->m_data + this->m_len_bytes, append_bits->get_len_bytes());
			}
			if (this->m_data != nullptr)
				delete[](this->m_data);
			this->m_data = new_data;
			this->m_len_bytes = new_len_bytes;
			this->m_len_bits += append_bits->get_len_bits();
			return true;
		}
		else
		{
			__ERROR("new uint8_t[%d] failed\n", new_len_bytes);
		}
	}
	return false;
}

cbits::~cbits()
{
	if (this->m_data != nullptr)
	{
		delete[](this->m_data);
		this->m_data = nullptr;
		this->m_len_bits = 0;
		this->m_len_bytes = 0;
	}	
}

cbits* cbits::bits_from_hex_file(string hex_file, READ_MODE endian)
{
	uint8_t *hex_data;
	int hex_data_len;
	int ret = cbits::read_file(hex_file, &hex_data, &hex_data_len);
	if (ret == 0 || hex_data_len == 0)
	{
		__ERROR("bits_form_hex_file(%s, %d) failed\n", hex_file.c_str(), endian);
		return nullptr;
	}
	
	cbits *ret_bits = new cbits(hex_data, hex_data_len, 0,  endian);
	delete[](hex_data);
	return ret_bits;
}

cbits* cbits::bits_from_bin_file(string bin_file, READ_MODE endian)
{
	ubit_t *bin_data;
	int hin_data_len;
	int ret = cbits::read_file(bin_file, (uint8_t**)&bin_data, &hin_data_len);
	if (ret == 0)
	{
		__ERROR("bits_form_bin_file(%s, %d) failed\n", bin_file.c_str(), endian);
		return nullptr;
	}
	cbits *ret_bits = new cbits(bin_data, hin_data_len, 1, endian);
	delete[](bin_data);
	return ret_bits;
}

ubit_t * cbits::hex_to_bits(uint8_t * hex_data, int len, READ_MODE endian)
{
	int bits_len = len * 8;
	ubit_t *st_bits = new ubit_t[bits_len];
	ubit_t *bits_copy = st_bits;
	if (endian == LEFT_BIG_ENDIAN)
	{
		for (int i = 0; i < len; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				*st_bits++ = (hex_data[i] >> (7 - j)) & 1;
			}
		}
	}
	if (endian == LEFT_LITTLE_ENDIAN)
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

uint8_t * cbits::bits_to_hex(ubit_t * bin_data, int len, READ_MODE endian)
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

	return hex_copy;
}

void cbits::unpack_bytes(uint8_t* btyes_out, int len, int* used_len)
{
	if (len > this->m_len_bytes)
	{
		len = this->m_len_bytes;
	}
	if (used_len != nullptr)
	{
		*used_len = len;
	}
	for (int i = 0; i < len; i++)
	{
		btyes_out[i] = this->unpack_i(i * 8, 8);
	}
}

void cbits::bits_reverse()
{
	for (int i = 0; i < this->m_len_bytes; i++)
	{
		this->m_data[i] ^= 0xFF;
	}
}

void cbits::bytes_reverse()
{
	for (int i = 0; i < this->m_len_bytes; i++)
	{
		uint8_t byte = this->m_data[i];
		this->m_data[i] = 0;
		this->m_data[i] |= (byte << 7) & 0x80;
		this->m_data[i] |= (byte << 5) & 0x40;
		this->m_data[i] |= (byte << 3) & 0x20;
		this->m_data[i] |= (byte << 1) & 0x10;
		this->m_data[i] |= (byte >> 1) & 0x08;
		this->m_data[i] |= (byte >> 3) & 0x04;
		this->m_data[i] |= (byte >> 5) & 0x02;
		this->m_data[i] |= (byte >> 7) & 0x01;
	}
}

cbits* cbits::truncate(int start, int len)
{
	if (start < 0)
	{
		start = 0;
	}
	if ((start + len) > this->m_len_bits)
	{
		len = this->m_len_bits - start;
	}
	ubit_t* bin_data = cbits::hex_to_bits(this->m_data, this->m_len_bytes, LEFT_BIG_ENDIAN);
	cbits *new_bits = new cbits(bin_data, len, true, this->m_endian);
	delete[](bin_data);
	return new_bits;
}

void cbits::save(string file, bool bits_mode)
{
	FILE* fp = nullptr;
	fopen_s(&fp, file.data(), "wb");
	if (fp == nullptr)
	{
		__ERROR("fopen(%s) failed\n", file.data());
		return;
	}

	if (bits_mode)
	{
		fwrite(this->m_data, 1, this->m_len_bytes, fp);
	}
	else
	{
		ubit_t *bin_data = cbits::hex_to_bits(this->m_data, this->m_len_bytes, LEFT_BIG_ENDIAN);
		fwrite(bin_data, 1, this->m_len_bits, fp);
		delete[](bin_data);
	}
	fclose(fp);
}

string cbits::hex_str()
{
	string ret_hex = string(m_len_bytes * 2 + 1, '\x00');
	int hex_str_len = this->m_len_bytes * 2 + 1;
	char* hex_str = (char*)ret_hex.data();

	for (int i = 0; i < this->m_len_bytes; i++)
	{
		sprintf(hex_str + 2 * i, "%02X", (uint8_t)this->unpack_i(i * 8, 8));
	}
	hex_str[hex_str_len - 1] = 0;
	return ret_hex;
}

string cbits::bin_str()
{
	string ret_hex = string(this->m_len_bits + 1, '\x00');
	char* bin_str = (char*)ret_hex.data();

	for (int i = 0; i < this->m_len_bits; i++)
	{
		bin_str[i] = '0' + (uint8_t)this->unpack_i(i, 1);
	}

	bin_str[this->m_len_bits - 1] = 0;
	return ret_hex;
}

string cbits::data_str()
{
	string ret_hex = string(m_len_bytes * 2 + 1, '\x00');
	int hex_str_len = this->m_len_bytes * 2 + 1;
	char* hex_str = (char*)ret_hex.data();

	for (int i = 0; i < this->m_len_bytes; i++)
	{
		sprintf(hex_str + 2 * i, "%02X", this->m_data[i]);
	}
	hex_str[hex_str_len - 1] = 0;
	return ret_hex;
}

int cbits::read_file(string file, uint8_t ** data, int *len)
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

int cframe::unpack_i(int start, int len)
{
	if (len > (sizeof(int) * 8))
	{
		__ERROR("len %d too long\n", len);
		return 0;
	}
	int used_len = 0;
	int ret = cbits::unpack_i(start, len, &used_len);
	if (used_len != len)
	{
		__ERROR("bits buffer exhaust, expect len=%d  get len=%d\n", len, used_len);
		return 0;
	}
	this->m_offset = start + used_len;
	return ret;
}

double cframe::unpack_d(int start)
{
	int used_len = 0;
	double ret = cbits::unpack_d(start, &used_len);
	if (used_len != (sizeof(double) * 8))
	{
		__ERROR("bits buffer exhaust, expect len=%d  get len=%d\n", 64, used_len);
		return 0;
	}
	this->m_offset = start + used_len;
	return ret;
}

float cframe::unpack_f(int start)
{
	int used_len = 0;
	float ret = cbits::unpack_f(start, &used_len);
	if (used_len != (sizeof(float) * 8))
	{
		__ERROR("bits buffer exhaust, expect len=%d  get len=%d\n", 32, used_len);
		return 0;
	}
	this->m_offset = start + used_len;
	return ret;
}

#if 1
void test1()
{
	cbits* test = new cbits("00110011110110101", 1);
	printf("[LEFT_BIG_ENDIAN]\n");
	printf("hex: %s\n", test->hex_str().c_str());
	//printf("unpack(1,7): 0x%X\n", test->unpack_i(1, 7));
	printf("unpack(1,8): 0x%X\n", test->unpack_i(1, 8));
	printf("unpack(2,3): 0x%X\n", test->unpack_i(2, 3));

	test->set_endian(LEFT_LITTLE_ENDIAN);
	printf("[LEFT_LITTLE_ENDIAN]\n");
	printf("hex: %s\n", test->hex_str().c_str());
	//printf("unpack(1,7): 0x%X\n", test->unpack_i(1, 7));
	printf("unpack(1,8): 0x%X\n", test->unpack_i(1, 8));
	printf("unpack(2,3): 0x%X\n", test->unpack_i(2, 3));

	delete(test);
}
int main()
{
	test1();
}

#endif
