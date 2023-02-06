#include "s19.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

int strtoi(char *str, int str_len, int radix)
{
	char *buffer = (char *)malloc(str_len + 1);
	if (buffer == NULL)
		return 0;

	memcpy(buffer, str, str_len);
	buffer[str_len] = 0;

	int ret = (int)strtol(buffer, NULL, radix);
	free(buffer);
	return ret;
}

uint8_t* strtobytes(char *str, int str_len)
{
	if (str_len & 1)
	{
		return NULL;
	}
	uint8_t *ret = (uint8_t *)malloc(str_len / 2);
	int i;

	for (i = 0; i < str_len / 2; i++)
	{
		ret[i] = strtoi(&str[i * 2], 2, 16);
	}
	
	return ret;
}

int s_record_checksum(char *s_record, int len)
{
	return 1;
}

int s19_parser(char *s19_file, char *output)
{
	FILE *fp_r, *fp_w;

	fopen_s(&fp_r, s19_file, "r");
	if (!fp_r)
	{
		printf("open %s failed\n", s19_file);
		return 0;
	}

	fopen_s(&fp_w, output, "wb");
	if (!fp_w)
	{
		printf("open %s failed\n", output);
		fclose(fp_r);
		return 0;
	}

	char line[256] = { 0 };
	int line_cnt = 0;
	while (fgets(line, sizeof(line), fp_r) != NULL)
	{
		if (strlen(line) == 0)
		{
			printf("no s-record\n");
			break;
		}	
		if (line[0] != 'S')
		{
			printf("line is not s-record format\n");
			break;
		}
		int len = strtoi(&line[2], 2, 16);
		int adr = 0;
		uint8_t *bytes = NULL;

		if (s_record_checksum(line, len) == 0)
		{
			printf("s-record checksum failed, break\n");
			break;
		}
		switch (line[1])
		{
			case '0':
				printf("S%c no implement\n", line[1]);
				break;
			case '1':
				adr = strtoi(&line[4], 4, 16);
				bytes = strtobytes(&line[8], (len - 2 - 1) * 2);
				fwrite(bytes, 1, len - 2 - 1, fp_w);
				line_cnt++;
				free(bytes);
				break;
			case '2':
				adr = strtoi(&line[4], 6, 16);
				bytes = strtobytes(&line[10], (len - 3 - 1) * 2);
				fwrite(bytes, 1, len - 3 - 1, fp_w);
				line_cnt++;
				free(bytes);
				break;
			case '3':
				adr = strtoi(&line[4], 8, 16);
				bytes = strtobytes(&line[12], (len - 4 - 1) * 2);
				fwrite(bytes, 1, len - 4 - 1, fp_w);
				line_cnt++;
				free(bytes);
				break;
			case '5':
				if (line_cnt != strtoi(&line[4], 4, 16))
				{
					printf("S1 S2 S3 line count error, %d != %d\n", line_cnt != strtoi(&line[4], 4, 16));
				}
				break;
			case '7':
				printf("S%c no implement\n", line[1]);
				break;
			case '8':
				printf("start address=0x%X\n", strtoi(&line[4], 6, 16));
				break;
			case '9':
				printf("S%c no implement\n", line[1]);
				break;
			default:
				printf("unknown s-record type: S%c\n", line[1]);
		}
	}

	fclose(fp_r);
	fclose(fp_w);
	return 1;
}