#include <algorithm>

#include "aud-dec/AudBuffer.h"


#undef ALIGN
#define ALIGN(value, alignment) (((value)+(alignment-1))&~(alignment-1))

void *malloc_aligned(size_t s, size_t align_to)
{
    char *pFull = (char*)malloc(s + align_to + sizeof(char *));
    char *pAlligned = (char *)ALIGN(((unsigned long)pFull + sizeof(char *)), align_to);

    *(char **)(pAlligned - sizeof(char*)) = pFull;

    return(pAlligned);
}

void free_aligned(void *p)
{
    if (!p)
        return;

    char *pFull = *(char **)(((char *)p) - sizeof(char *));
    free(pFull);
}

OMXPacket *AllocPacket(int size)
{
    OMXPacket *pkt = (OMXPacket *)malloc(sizeof(OMXPacket));
    if (pkt) {
        memset(pkt, 0, sizeof(OMXPacket));

        pkt->data = (uint8_t*) malloc(size + 16);
        if(!pkt->data) {
            free(pkt);
            pkt = NULL;
        }
        else {
            memset(pkt->data + size, 0, 16);
            pkt->size = size;
        }
    }
    return pkt;
}

void FreePacket(OMXPacket *pkt)
{
    if (pkt) {
        if(pkt->data) {
            free(pkt->data);
            pkt->data = NULL;
        }
        free(pkt);
        pkt = NULL;
    }
}
