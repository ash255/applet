#ifndef __BITS_H__
#define __BITS_H__
#include <stdint.h>
#include <string>
#include "logger.h"
using std::string;
typedef uint8_t ubit_t;

#ifndef __ERROR
#define __ERROR printf
#endif

//typedef cbits sx_bits;
//typedef cframe sx_frame;
#define sx_bits cbits
#define sx_frame cframe

/*
	为了节省内存空间，所有的数据将以字节的形式保存，并且是从左到右大端序的形式，不足的部分补0
	例如：比特串为00110011 1101101，保存的数据为0x33, 0xDA
	不同的读取方式将导致得到不同的结果，目前主要有两种读取方式，LEFT_BIG_ENDIAN和LEFT_LITTLE_ENDIAN
	在LEFT_BIG_ENDIAN模式下，unpack(8)将得到0x33, unpack(3)将得到1
	在LEFT_LITTLE_ENDIAN模式下，unpack(8)将得到0xCC, unpack(3)将得到4
*/

enum READ_MODE
{
	LEFT_BIG_ENDIAN,
	LEFT_LITTLE_ENDIAN,
};

class cbits
{
public:
	cbits(READ_MODE endian = LEFT_BIG_ENDIAN) :m_endian(endian), m_data(nullptr), m_len_bytes(0), m_len_bits(0) {}
	cbits(uint8_t* data, int len, bool bits_mode = true, READ_MODE endian = LEFT_BIG_ENDIAN);
	cbits(string str, bool bits_mode = true, READ_MODE endian = LEFT_BIG_ENDIAN);
	~cbits();

	/* static methods */
	static cbits* bits_from_hex_file(string hex_file, READ_MODE endian = LEFT_BIG_ENDIAN);
	static cbits* bits_from_bin_file(string bin_file, READ_MODE endian = LEFT_BIG_ENDIAN);
	static ubit_t* hex_to_bits(uint8_t* hex_data, int len, READ_MODE endian);
	static uint8_t* bits_to_hex(ubit_t* bin_data, int len, READ_MODE endian);
	static int memmem(ubit_t* src, int src_len, ubit_t* search, int search_len);

	/* read methods */
	int unpack_i(int start, int len, int* used_len = nullptr) { return this->unpack<int>(start, len, used_len); }
	int64_t unpack_l(int start, int len, int* used_len = nullptr) { return this->unpack<int64_t>(start, len, used_len); }
	double unpack_d(int start, int* used_len = nullptr) { int64_t r = this->unpack_l(start, sizeof(double) * 8, used_len); return *(double*)&r; }
	float unpack_f(int start, int* used_len = nullptr) { int64_t r = this->unpack_l(start, sizeof(float) * 8, used_len); return *(float*)&r; }
	void unpack_bytes(uint8_t* btyes_out, int len, int* used_len = nullptr);

	/* operate methods */
	void bits_reverse();
	void bytes_reverse();
	cbits* truncate(int start, int len);
	bool append(cbits* append_bits);
	cbits* copy() { return new cbits(this->m_data, this->m_len_bytes, false, this->m_endian); }
	void save(string file, bool bits_mode = true);

	/* string format output */
	string hex_str();
	string bin_str();
	string data_str();

	/* get/set methods */
	int get_len_bytes() { return m_len_bytes; }
	int get_len_bits() { return m_len_bits; }
	uint8_t* get_data() { return m_data; }
	READ_MODE get_endian() { return m_endian; }
	void set_endian(READ_MODE endian) { this->m_endian = endian; }
	ubit_t *get_data_bits(int *bits_len = nullptr);
protected:
	uint8_t *m_data;
	int m_len_bytes;
	int m_len_bits;
	READ_MODE m_endian;

	static int read_file(string file, uint8_t **data, int *len);
	template<typename type> type unpack(int start, int len, int*used_len = nullptr)
	{
		type tmp = 0;

		if (start < 0)
		{
			start = 0;
		}
		if (this->m_endian == LEFT_BIG_ENDIAN || this->m_endian == LEFT_LITTLE_ENDIAN)
		{
			if ((start + len) > this->m_len_bits)
			{
				len = this->m_len_bits - start;
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
		uint8_t* pData = this->m_data + start / 8;
		if (this->m_endian == LEFT_BIG_ENDIAN)
		{
			int move_bits = start % 8;
			for (int i = 0; i < len; i++)
			{
				tmp <<= 1;
				tmp |= ((*pData >> (7 - move_bits++)) & 1);
				if (move_bits == 8)
				{
					pData++;
					move_bits = 0;
				}
			}
			return tmp;
		}
		else if (this->m_endian == LEFT_LITTLE_ENDIAN)
		{
			int move_bits = start % 8;
			int offset = 0;
			for (int i = 0; i < len; i++)
			{
				tmp |= (type)((*pData >> (7 - move_bits++)) & 1) << offset++;
				if (move_bits == 8)
				{
					pData++;
					move_bits = 0;
				}
			}
			return tmp;
		}
		return 0;
	}
};

class cframe : public cbits
{
public:
	cframe(READ_MODE endian = LEFT_BIG_ENDIAN, int offset = 0) :m_offset(offset), cbits(endian) {}
	cframe(uint8_t* data, int len, bool bits_mode = true, READ_MODE endian = LEFT_BIG_ENDIAN, int offset = 0) :m_offset(offset), cbits(data, len, bits_mode, endian) {}
	cframe(string str, bool bits_mode = true, READ_MODE endian = LEFT_BIG_ENDIAN, int offset = 0) :m_offset(offset), cbits(str, bits_mode, endian) {}
	cframe(cbits* bits, int offset = 0);

	/* overwrite methods */
	int unpack_i(int len) { return this->unpack_i(this->m_offset, len);}
	int unpack_i(int start, int len);
	double unpack_d() { return this->unpack_d(this->m_offset); }
	double unpack_d(int start);
	float unpack_f() { return this->unpack_f(this->m_offset); }
	float unpack_f(int start);
	cframe* copy() { return new cframe(this->m_data, this->m_len_bytes, false, this->m_endian, this->m_offset); }
	cframe* truncate(int start, int len) { cbits *bits = cbits::truncate(start, len); cframe *frame = new cframe(bits); delete(bits); return frame; }

	/* peek methods */
	int peek_i(int len) { return this->peek_i(this->m_offset, len); }
	int peek_i(int start, int len) { return this->unpack_i(start, len); }
	double peek_d() { return this->peek_d(this->m_offset); }
	double peek_d(int start) { return this->unpack_d(start); }
	float peek_f() { return this->peek_f(this->m_offset); }
	float peek_f(int start) { return this->unpack_f(start); }

	/* string format output */
	string left_bits_str() { return this->left_bits_str(this->m_offset, this->get_left_len()); }
	string left_bits_str(int start, int len) { return this->bin_str().substr(start, len); }

	/* get/set methods */
	int get_offset() { return this->m_offset; }
	void set_offset(int offset) { offset <= this->get_len_bits() ? this->m_offset = offset : __ERROR("offset > bits_len, offset=%d bits_len=%d\n", offset, this->get_len_bits()); }
	int get_left_len() { return this->m_len_bits - this->m_offset; }
private:
	int m_offset;
};

class sx_mcs
{
public:
	uint8_t mcs_id;
	uint16_t cw;
};

class sx_status
{
public:
	int m_sfid;
	int m_rlc_mode;
	int num_mcs;
	sx_mcs mcs[15];
};

#endif
