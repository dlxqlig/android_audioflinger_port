#pragma once

#include <stdio.h>
#include <string.h>

#include "System.h"

typedef struct OMXPacket
{
    int size;
    uint8_t *data;
} OMXPacket;

void *malloc_aligned(size_t s, size_t alignTo);
void free_aligned(void *p);
OMXPacket *AllocPacket(int size);
void FreePacket(OMXPacket *pkt);
