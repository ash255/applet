#include "pcapng.h"
#include "logger.h"
#include <stdio.h>

int main()
{
    FILE *fp = NULL;
    pcapng_t *pcap = NULL;
    pcapng_block_t *block = NULL;

    log_with_fd(stdout);
    log_set_level(__LEVEL_DEBUG__);
    log_set_level(__LEVEL_MESSAGE__);
    log_set_level(__LEVEL_ERROR__);

    fp = fopen("/home/sdr/tcpdump/ims_20211231.pcapng", "rb");
    if(fp == NULL)
    {
        __LOG_ERROR__("main", "open pcapng failed\n");
        return 0;
    }
    pcap = pcapng_analyse(fp);

    while (1)
    {
        block = pcapng_block_next(pcap);
        if(block == NULL)
        {
            break;
        }
        if (block->block_type == pcapng_type_enhanced_packet)
        {
            pcapng_enhanced_packet_t *packet = (pcapng_enhanced_packet_t *)block;
            __LOG_MESSAGE_ARRAY__("main", packet->data, packet->len_packet);
        }
        pcapng_block_free(block);
    }
    pcapng_block_free(pcap);
    fclose(fp);
    return 0;
}