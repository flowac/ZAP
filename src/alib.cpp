#include "alib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pt = boost::posix_time;

inline time_t sNow()
{
    time_t raw;
    time(&raw);
    return raw;
    return pt::second_clock::universal_time();
}

inline void printTime(time_t time)
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
    for (uint32_t i = 0; i < nPack; i++) {
//        printf("%lX\t", target->packs[i]);
    }
    printf("\n");
}

pack *newPack(char *dn, uint64_t xl, char *xt, char *tr)
{
    uint32_t ndn = strlen(dn) + 1;
    uint32_t nxt = strlen(xt) + 1;
    uint32_t ntr = strlen(tr) + 1;
    
    if (ndn > MAX_U8 || nxt > MAX_U8 || ntr > MAX_U8) 
	return NULL;
    
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
    if (bx == NULL) 
	return NULL;

    bx->time = (uint32_t)sNow();
    bx->key = key;
    if (nPack < MAX_U16) {
        bx->nPack = nPack;
        bx->packs = packs;
    } else {
        bx->nPack = MAX_U16;
        for (uint32_t i = MAX_U16; i < nPack; i++) {
            free(packs[i]);
        }
        bx->packs = (pack **)realloc(packs, sizeof(pack *) * MAX_U16);
    }
    bx->nTran = 0;
    bx->trans = NULL;

    if (LOG) 
	printTime(sNow());

    return bx;
}

chain *newChain(void)
{
    chain *ch = (chain *)malloc(sizeof(chain));
    if (!ch) 
	return NULL;
    
    ch->size = 0;
    ch->head = NULL;
    return ch;
}

//! return 1 on success
bool insertBlock(block *bx, chain *ch)
{
    ch->head = (block **)realloc(ch->head, sizeof(block *) * (ch->size + 1));
    if (!ch->head)
	return 0;
    
    ch->head[ch->size] = bx;//! Don't free block pointer
    ch->size++;
    ch->time = (uint32_t)sNow();
    return true;
}

uint32_t deletePack(pack *target)
{
    uint32_t bytesFreed = sizeof(pack);
    
    if (target->dn != NULL) {
        bytesFreed += strlen(target->dn) + 1;
        free(target->dn);
    }
    
    if (target->xt != NULL) {
        bytesFreed += strlen(target->xt) + 1;
        free(target->xt);
    }
    
    if (target->tr != NULL) {
        bytesFreed += strlen(target->tr) + 1;
        free(target->tr);
    }
    
    return bytesFreed;
}

uint32_t deleteBlock(block *target)
{
    uint32_t i, bytesFreed = sizeof(block);
    if (target->packs != NULL && target->nPack > 0) {
        for (i = 0; i < target->nPack; i++) {
            bytesFreed += deletePack(target->packs[i]) + sizeof(pack *);
            free(target->packs[i]);
        }
        free(target->packs);
    }

    if (target->trans != NULL && target->nTran > 0) {
        for (i = 0; i < target->nTran; i++) {
            if (target->trans[i] != NULL) {
                bytesFreed += sizeof(tran *);
                free(target->trans[i]);
            }
        }
        free(target->trans);
    }

    return bytesFreed;
}

uint32_t deleteChain(chain *target)
{
    uint32_t bytesFreed = 0;
    for (uint32_t i = 0; i < target->size; i++) {
        bytesFreed += deleteBlock(target->head[i]) + sizeof(block *);
        free(target->head[i]);
    }
    
    free(target->head);
    return bytesFreed;
}

