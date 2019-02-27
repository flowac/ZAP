#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include "alib.h"
#include "lzma_wrapper.h"
#include "log.h"

time_t sNow()
{
    time_t raw;
    time(&raw);
    return raw;
}

void printTime(time_t time)
{
	char buf[256];
	strftime(buf, 256, "%g %B %d  %H:%M:%S", localtime(&time));
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
    uint8_t i;
    
    if (ndn > MAX_U8 || nxt > MAX_U8 || ntr > MAX_U8) 
        return NULL;
    
    pack *px = (pack *)malloc(sizeof(pack));
    if (!px)
        return NULL; // maloc failed

    px->xl = xl;
    for (i = 0; i < 6; i++) {
        px->info[i] = 0; // prevent valgrind errors
    }
    strncpy(px->info, dn, 5);
    
    px->dn = (char *)malloc(sizeof(char) * ndn);
    if (!px->dn)
	goto cleanup;
    strcpy(px->dn, dn);
    
    px->xt = (char *)malloc(sizeof(char) * nxt);
    if (!px->xt)
	goto cleanup;
    strcpy(px->xt, xt);
    
    px->tr = (char *)malloc(sizeof(char) * ntr);
    if (!px->tr)
	goto cleanup;
    strcpy(px->tr, tr);
    return px;
    
 cleanup:
    log_msg_default;
    deletePack(px);
    free(px);
    return NULL;
}

tran *newTran()
{
    return NULL;
}

block *newBlock(uint32_t n, uint64_t key, uint32_t nPack, pack **packs)
{
    block *bx = (block *)malloc(sizeof(block));
    if (bx == NULL) {
	return NULL;
        log_msg_default;
    }

    bx->time = (uint32_t)sNow();
    bx->crc = 0;
    bx->n = n;
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

block *restore_block(uint32_t time, uint32_t crc, uint16_t n_pack,
                     uint16_t n_tran, uint32_t n, uint64_t key,
                     pack **packs)
{
    block *bx = (block *)malloc(sizeof(block));
    if (bx == NULL){
        log_msg_default;
        return NULL;
    }
    bx->time = time;
    bx->crc = crc;
    bx->nPack = n_pack;
    bx->nTran = n_tran;
    bx->n = n;
    bx->key = key;
    if (n_pack < MAX_U16) {
        bx->nPack = n_pack;
        bx->packs = packs;
    } else {
        bx->nPack = MAX_U16;
        for (uint32_t i = MAX_U16; i < n_pack; i++) {
            free(packs[i]);
        }
        bx->packs = (pack **)realloc(packs, sizeof(pack *) * MAX_U16);
    }
    bx->nTran = 0;
    bx->trans = NULL;
    return bx;
}

chain *newChain(void)
{
    chain *ch = (chain *)malloc(sizeof(chain));
    if (ch == NULL) {
        log_msg_default;
	return NULL;
    }
    
    ch->size = 0;
    ch->head = NULL;
    return ch;
}

//! return 1 on success
bool insertBlock(block *bx, chain *ch)
{
    if (bx == NULL) return 0;
    block **tmp = (block **)realloc(ch->head, sizeof(block *) * (ch->size + 1));

    // if realloc was successfull assign to head
    if (tmp != NULL)
        ch->head = tmp;
    else {
        log_msg_default;
        return 0;
    }
    
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

