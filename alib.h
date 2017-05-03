#ifndef _ALIB_H
#define _ALIB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "atype.h"

namespace pt = boost::posix_time;

inline time_t sNow()
{
    time_t rawT;
    time(&rawT);
    return rawT;
//    return pt::second_clock::universal_time();
}

//! Only gets the milliseconds part of the current time
inline uint16_t msNow()
{
return 0;
//    return pt::microsec_clock::universal_time().total_milliseconds();
}

void printTime(time_t time)
{
    pt::ptime tout = pt::from_time_t(time);
    std::cout << tout << std::endl;
}

void printBlock(block *target)
{
    uint32_t nPack = target->nPack;

    printTime((time_t) target->time);
    printf("[%u] > 0x%lX [%u]\n",
           target->time, target->key, nPack);

    if (!LOG) return;
    for (uint32_t i = 0; i < nPack; i++)
    {
//        printf("%lX\t", target->packs[i]);
    }
    printf("\n");
}

pack *newPack(char *dn, uint64_t xl, char *xt, char *tr)
{
    uint32_t ndn = strlen(dn) + 1;
    uint32_t nxt = strlen(xt) + 1;
    uint32_t ntr = strlen(tr) + 1;
    
    if (ndn > MAX_U8 || nxt > MAX_U8 || ntr > MAX_U8) return NULL;
    
    pack *px = (pack *)malloc(sizeof(pack));
    px->xl = xl;
    strncpy(px->info, dn, sizeof(px->info) / sizeof(char) - 1);
    
    px->dn = (char *)malloc(sizeof(char) * ndn);
    strcpy(px->dn, dn);
    
    px->xt = (char *)malloc(sizeof(char) * nxt);
    strcpy(px->xt, xt);
    
    px->tr = (char *)malloc(sizeof(char) * ntr);
    strcpy(px->tr, tr);
    
    return px;
}

tran *newTran()
{
    return NULL;
}

block *newBlock(uint64_t key, uint32_t nPack, pack **packs)
{
    block *bx = (block *)malloc(sizeof(block));

    bx->time = (uint32_t)sNow();
    bx->key = key;
    if (nPack < MAX_U16)
    {
        bx->nPack = nPack;
        bx->packs = packs;
    }
    else
    {
        uint32_t i;
        bx->nPack = MAX_U16;
        for (i = MAX_U16; i < nPack; i++)
        {
            free(packs[i]);
        }
        if (NULL == realloc(packs, sizeof(pack *) * MAX_U16))
        {
            for (i = 0; i < MAX_U16; i++)
            {
                free(packs[i]);
            }
            free(packs);
            free(bx);
            return NULL;
        }
        bx->packs = packs;
    }
    bx->nTran = 0;

    if (LOG) printTime(sNow());
    return bx;
}

chain *newChain(void)
{
    chain *ch = (chain *)malloc(sizeof(chain));
    ch->size = 0;
    ch->head = NULL;
    return ch;
}

//! return 1 on success
bool insertBlock(block *bx, chain *ch)
{
    void *tmp = realloc(ch->head, sizeof(block *) * (ch->size + 1));
    if (tmp == NULL) return 0;

    ch->head = (block **)tmp;
    ch->head[ch->size] = bx;//! Don't free block pointer
    ch->size++;
    ch->time = (uint32_t)sNow();
    return 1;
}

uint32_t deletePack(pack *target)
{
    uint32_t bytesFreed = 0;
    
    if (target->dn != NULL)
    {
        bytesFreed += sizeof(target->dn) / sizeof(char);
        free(target->dn);
    }
    
    if (target->xt != NULL)
    {
        bytesFreed += sizeof(target->xt) / sizeof(char);
        free(target->xt);
    }
    
    if (target->tr != NULL)
    {
        bytesFreed += sizeof(target->tr) / sizeof(char);
        free(target->tr);
    }
    
    return bytesFreed;
}

uint32_t deleteBlock(block *target)
{
    uint32_t i, bytesFreed = 0;
    if (target->packs != NULL && target->nPack > 0)
    {
        for (i = 0; i < target->nPack; i++)
        {
            bytesFreed += deletePack(target->packs[i]);
            free(target->packs[i]);
        }
        free(target->packs);
    }

    if (target->trans != NULL && target->nTran > 0)
    {
        for (i = 0; i < target->nTran; i++)
        {
            if (target->trans[i] != NULL)
            {
                bytesFreed += sizeof(tran);
                free(target->trans[i]);
            }
        }
//        bytesFreed += target->nTran * sizeof(tran);
        free(target->trans);
    }

    return bytesFreed + sizeof(pack **) + sizeof(tran *);
}

uint32_t deleteChain(chain *target)
{
    uint32_t bytesFreed = 0;
    for (uint32_t i = 0; i < target->size; i++)
    {
        bytesFreed += deleteBlock(target->head[i]);
        free(target->head[i]);
        bytesFreed += sizeof(block);
    }
    free(target->head);
    return bytesFreed + sizeof(block **);

    //! And then free the chain pointer from the caller
}

#endif//_ALIB_H

