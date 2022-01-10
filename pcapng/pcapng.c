#include "pcapng.h"
#include "logger.h"
#include <string.h>
#include <stdlib.h>

pcapng_t *pcapng_analyse(FILE *fp)
{
    if (fp == NULL)
    {
        __LOG_ERROR__("pcapng_analyse", "fp NULL\n");
        return NULL;
    }

    pcapng_t *pcap = (pcapng_t *)malloc(sizeof(pcapng_t));
    if (pcap == NULL)
    {
        __LOG_ERROR__("pcapng_analyse", "malloc failed\n");
        return NULL;
    }

    pcap->fp = fp;
    pcap->offset = ftell(fp);
    pcap->block_count = 0;

    return pcap;
}

pcapng_block_t *pcapng_block_next(pcapng_t *pcap)
{
    uint32_t block_type, block_length;
    pcapng_block_t *block = NULL;

    if (pcap == NULL)
    {
        __LOG_ERROR__("pcapng_next", "pcap NULL\n");
        goto __LABEL_PCAPNG_NEXT_FAILED;
    }

    if (feof(pcap->fp))
    {
        return NULL;
    }

    if (fread(&block_type, 1, sizeof(uint32_t), pcap->fp) != sizeof(uint32_t))
    {
        //__LOG_ERROR__("pcapng_next", "read block_type failed\n");
        goto __LABEL_PCAPNG_NEXT_FAILED;
    }

    if (fread(&block_length, 1, sizeof(uint32_t), pcap->fp) != sizeof(uint32_t))
    {
        //__LOG_ERROR__("pcapng_next", "read block_length failed\n");
        goto __LABEL_PCAPNG_NEXT_FAILED;
    }

    block = (pcapng_block_t *)malloc(block_length);
    block->block_type = block_type;
    block->block_length = block_length;

    if (fread(&block->data, 1, (block_length - 2 * sizeof(uint32_t)), pcap->fp) != (block_length - 2 * sizeof(uint32_t)))
    {
        __LOG_ERROR__("pcapng_next", "read data failed\n");
        goto __LABEL_PCAPNG_NEXT_FAILED;
    }

    pcap->block_count++;
    return block;

__LABEL_PCAPNG_NEXT_FAILED:
    if (block != NULL)
    {
        free(block);
    }
    return NULL;
}

void pcapng_block_free(pcapng_block_t *block)
{
    if (block != NULL)
    {
        free(block);
    }
}

void pcapng_free(pcapng_t *pcap)
{
    if (pcap != NULL)
    {
        free(pcap);
    }
}
