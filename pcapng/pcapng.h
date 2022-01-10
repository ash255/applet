#ifndef __pcapNG_H__
#define __pcapNG_H__
#include <stdint.h>
#include <stdio.h>

/* 以下很多宏定义和结构体参考了010 editor */
#define pcapng_type_enhanced_packet 6

typedef struct pcapng_enhanced_packet_t
{
    uint32_t block_type;
    uint32_t block_length;
    uint32_t interface_id;
    uint64_t timestamp;
    uint32_t len_capture;
    uint32_t len_packet;
    uint8_t data[0];
}__attribute__((packed)) pcapng_enhanced_packet_t;

typedef struct pcapng_block_t
{
    uint32_t block_type;
    uint32_t block_length;
    uint8_t data[0];
}pcapng_block_t;

typedef struct pcapng_t
{
    FILE *fp;
    long offset;
    long block_count;
}pcapng_t;

pcapng_t* pcapng_analyse(FILE *fp);
pcapng_block_t* pcapng_block_next(pcapng_t *pcap);

void pcapng_block_free(pcapng_block_t *block);
void pcapng_free(pcapng_t *pcap);

#endif