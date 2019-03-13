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
	char buf[MAX_U8];
	strftime(buf, MAX_U8, "%g %B %d  %H:%M:%S", localtime(&time));
}

void printBlock(block *target)
{
    uint32_t n_packs = target->n_packs;

    printTime((time_t) target->time);
    printf("[%u] > 0x%lX [%u]\n",
           target->time, target->key, n_packs);

    if (!LOG) return;
    for (uint32_t i = 0; i < n_packs; i++) {
//        printf("%lX\t", target->packs[i]);
    }
    printf("\n");
}

bool newPack(pack *px, char *dn, uint64_t xl, char *xt, char *tr)
{
    uint32_t ndn = strlen(dn) + 1;
    uint32_t nxt = strlen(xt) + 1;
    uint32_t ntr = strlen(tr) + 1;
    uint8_t i;
    
    if (ndn > MAX_U8 || nxt > MAX_U8 || ntr > MAX_U8) 
        return 1;
    
    px->xl = xl;
    for (i = 0; i < 6; i++) {
        px->info[i] = 0; // prevent valgrind errors
    }
    strncpy(px->info, dn, 5);
    
    px->dn = (char *)malloc(sizeof(char) * ndn);
    if (!px->dn)
        return 1;
    strcpy(px->dn, dn);
    
    px->xt = (char *)malloc(sizeof(char) * nxt);
    if (!px->xt)
        return 1;
    strcpy(px->xt, xt);
    
    px->tr = (char *)malloc(sizeof(char) * ntr);
    if (!px->tr)
        return 1;
    strcpy(px->tr, tr);
    return 0;
}

void newTran(tran *tx)
{
}

void newBlock(block *bx, uint32_t n, uint64_t key, uint32_t *n_packs, pack **packs)
{
    bx->time = (uint32_t)sNow();
    bx->crc = 0;
    bx->n = n;
    bx->key = key;
    bx->packs = *packs;
    if (*n_packs > MAX_U16) {
        bx->n_packs = MAX_U16;
	*n_packs -= MAX_U16;
	*packs = &((*packs)[MAX_U16]);
    } else {
    	bx->n_packs = *n_packs;
	*n_packs = 0;
        *packs = 0;
    }
    bx->n_trans = 0;
    bx->trans = 0;
    if (LOG) printTime(bx->time);
}

void restore_block(block *bx, uint32_t time, uint32_t crc, uint16_t *n_packs,
                     uint16_t n_trans, uint32_t n, uint64_t key,
                     pack **packs)
{
    bx->time = time;
    bx->crc = crc;
    bx->n_packs = *n_packs;
    bx->n_trans = n_trans;
    bx->n = n;
    bx->key = key;
    if (*n_packs > MAX_U16) {
        bx->n_packs = MAX_U16;
	*n_packs -= MAX_U16;
	*packs = &((*packs)[MAX_U16]);
    } else {
    	bx->n_packs = *n_packs;
	*n_packs = 0;
        *packs = 0;
    }
    bx->n_trans = 0;
    bx->trans = NULL;
}

chain *newChain(void)
{
    chain *ch = (chain *)malloc(sizeof(chain));
    if (ch == NULL) {
        log_msg_default;
	return NULL;
    }
    
    ch->n_bal = 0;
    ch->n_blk = 0;
    ch->bal  = 0;
    memset(ch->blk, 0, sizeof(block *) * 2000);
    return ch;
}

//! return 1 on success
//TODO: compact blocks once full
bool insertBlock(block *bx, chain *ch)
{
    if (bx == 0 || ch == 0) return 0;

    if (ch->n_blk < B_SUM) {
        ch->blk[ch->n_blk] = *bx;
	ch->n_blk++;
    } else {
        log_msg_default;
        return 0;
    }
    return true;
}

void deletePack(pack *target)
{
    if (target->dn != NULL) free(target->dn);
    if (target->xt != NULL) free(target->xt);
    if (target->tr != NULL) free(target->tr);
}

void deleteBlock(block *target)
{
    uint32_t i;
    if (target == 0) return;
    for (i = 0; i < target->n_packs; i++) {
        free(target->packs[i].dn);
        free(target->packs[i].xt);
        free(target->packs[i].tr);
    }
    if (target->packs != 0) free(target->packs);
    if (target->trans != 0) free(target->trans);
}

void deleteChain(chain *target)
{
    uint32_t i;
    for (i = 0; i < B_SUM; i++) deleteBlock(&(target->blk[i]));
    free(target->bal);
}

