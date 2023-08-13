#ifndef __BITS_H__
#define __BITS_H__
#include <stdint.h>
#include <string>
using std::string;
typedef uint8_t ubit_t;

/*
	以比特序列001100111101101为例
	由于长度不为8的倍数，在转换为字节时需要补充0，补充0的位置有4种情况
	1. LEFT_BIG_ENDIAN
		比特序列从左开始大端处理，将序列分成00110011 1101101[0]，转换为字节为0x33, 0xDA
	2. LEFT_LITTLE_ENDIAN
		比特序列从左开始小端处理，将序列分成00110011 [0]1101101，转换为字节为0xCC, 0xB6
	3. RIGHT_BIG_ENDIAN
		比特序列从右开始大端处理，将序列分成0011001[0] 11101101，转换为字节为0x32, 0xED
	4. RIGHT_LITTLE_ENDIAN
		比特序列从右开始小端处理，将序列分成[0]0011001 11101101，转换为字节为0x98, 0xB7
	注：当序列长度为8的倍数时，LEFT_BIG_ENDIAN和RIGHT_BIG_ENDIAN结果相同，LEFT_LITTLE_ENDIAN和RIGHT_LITTLE_ENDIAN结果相同
*/
enum BITS_ENDIAN
{
	LEFT_BIG_ENDIAN,
	LEFT_LITTLE_ENDIAN,
	RIGHT_BIG_ENDIAN,
	RIGHT_LITTLE_ENDIAN
};

class st_bits
{
public:
	st_bits(BITS_ENDIAN endian = LEFT_BIG_ENDIAN) :m_endian(endian) { this->m_bits = nullptr; this->m_len = 0; }
	st_bits(uint8_t *data, int len, int bits_mode = 1, BITS_ENDIAN endian = LEFT_BIG_ENDIAN);
	st_bits(string str, int bits_mode = 1, BITS_ENDIAN endian = LEFT_BIG_ENDIAN);
	~st_bits();

	static st_bits* bits_form_hex_file(string hex_file, BITS_ENDIAN endian = LEFT_BIG_ENDIAN);
	static st_bits* bits_form_bin_file(string bin_file, BITS_ENDIAN endian = LEFT_BIG_ENDIAN);
	static ubit_t* hex_to_bits(uint8_t *hex_data, int len, BITS_ENDIAN endian);
	static uint8_t* bits_to_hex(ubit_t *bin_data, int len, BITS_ENDIAN endian);
	static int memmem(ubit_t *src, int src_len, ubit_t *search, int search_len);

	int unpack_i(int start, int len, int*used_len = nullptr) { return this->unpack<int>(start, len, used_len); }
	int64_t unpack_l(int start, int len, int*used_len = nullptr) { return this->unpack<int64_t>(start, len, used_len); }
	double unpack_d(int start, int*used_len = nullptr) { int64_t r = this->unpack_l(start, sizeof(double) * 8, used_len); return *(double*)&r; }
	float unpack_f(int start, int*used_len = nullptr) { int64_t r = this->unpack_l(start, sizeof(float) * 8, used_len); return *(float*)&r; }
	
	void bits_reverse();
	int search(ubit_t *bits, int len, int start = 0) { if (start > this->m_len) { return 0; } return st_bits::memmem(&this->m_bits[start], this->m_len - start, bits, len); }
	int search(string bits, int start=0);
	st_bits* truncate(int start, int len);

	string hex_str();
	string bin_str(int start = 0);
	
	int get_len() { return m_len; }
	ubit_t *get_bits() { return m_bits; }
private:
	ubit_t *m_bits;
	int m_len;
	BITS_ENDIAN m_endian;

	static int read_file(string file, uint8_t **data, int *len);
	template<typename type> type unpack(int start, int len, int*used_len = nullptr)
	{
		uint8_t offset = 0;
		type tmp = 0;

		if (start < 0)
		{
			start = 0;
		}
		if (this->m_endian == LEFT_BIG_ENDIAN || this->m_endian == LEFT_LITTLE_ENDIAN)
		{
			if ((start + len) > this->m_len)
			{
				len = this->m_len - start;
			}
		}
		else
		{
			if ((start - len) < 0)
			{
				len = start;
			}
		}
		if (len > (sizeof(type) * 8))
		{
			len = sizeof(type) * 8;
		}
		if (used_len != nullptr)
		{
			*used_len = len;
		}
		if (this->m_endian == LEFT_BIG_ENDIAN)
		{
			for (int i = start; i < start + len; i++)
			{
				tmp <<= 1;
				tmp |= (type)this->m_bits[i];
			}
			return tmp;
		}
		else if (this->m_endian == LEFT_LITTLE_ENDIAN)
		{
			for (int i = start; i < start + len; i++)
			{
				tmp |= (type)this->m_bits[i] << offset++;
			}
			return tmp;
		}
		else if (this->m_endian == RIGHT_BIG_ENDIAN)
		{
			for (int i = start; i >(start - len); i--)
			{
				tmp |= (type)this->m_bits[i] << offset++;
			}
			return tmp;
		}
		else if (this->m_endian == RIGHT_LITTLE_ENDIAN)
		{
			for (int i = start; i > (start - len); i--)
			{
				tmp <<= 1;
				tmp |= (type)this->m_bits[i];
			}
			return tmp;
		}
		return 0;
	}
};

class st_frame
{
public:
	st_frame(st_bits *b, int start=0) :m_bits(b), m_start(start){};

	st_frame *copy() { return new st_frame(this->m_bits, this->m_start); }
	int unpack_i(int len) { return this->unpack_i(this->m_start, len); }
	int unpack_i(int start, int len);
	double unpack_d() { return this->unpack_d(this->m_start); }
	double unpack_d(int start);
	float unpack_f() { return this->unpack_f(this->m_start); }
	float unpack_f(int start);

	int peek_i(int len) { return this->peek_i(this->m_start, len); }
	int peek_i(int start, int len) { return this->m_bits->unpack_i(start, len); }
	double peek_d() { return this->peek_d(this->m_start); }
	double peek_d(int start) { return this->m_bits->unpack_d(start); }
	float peek_f() { return this->peek_f(this->m_start); }
	float peek_f(int start) { return this->m_bits->unpack_f(start); }

	void set_start(int start = 0);
	int left_len() { return m_bits->get_len() - this->m_start; }
	int handle_len() { return this->m_start; }
	string get_bits_str() { return this->m_bits->bin_str(); }
	string get_left_bits_str() { return this->m_bits->bin_str(this->m_start); }
private:
	st_bits *m_bits;
	int m_start;
};

#endif
