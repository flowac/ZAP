#include "alib.h"
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../extern/7z/LzmaAlone.h"

namespace pt = boost::posix_time;

inline time_t sNow()
{
    time_t raw;
    time(&raw);
    return raw;
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
    uint8_t i;
    
    if (ndn > MAX_U8 || nxt > MAX_U8 || ntr > MAX_U8) 
        return NULL;
    
    pack *px = (pack *)malloc(sizeof(pack));
    if (!px)
        return NULL; // maloc failed

    px->xl = xl;
    for (i = 0; i < 6; i++) {px->info[i] = 0;}//prevent valgrind errors
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
    deletePack(px);
    free(px);
    return NULL;
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
    bx->crc = 0;
    bx->n = 0;
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
    block **tmp = (block **)realloc(ch->head, sizeof(block *) * (ch->size + 1));

    // if realloc was successfull assign to head
    if (tmp != NULL)
        ch->head = tmp;
    else
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

void packToText(pack *pk, FILE *fp, char *buf, int len)
{
    //3 tabs
    snprintf(buf, len, "\t\t{\
\n\t\t\tPinfo: %s,\
\n\t\t\tPdn  : %s,\
\n\t\t\tPxl  : %ld,\
\n\t\t\tPxt  : %s,\
\n\t\t\tPtr  : %s,\
\n\t\t},\n", pk->info, pk->dn, pk->xl, pk->xt, pk->tr);

    fputs(buf, fp);
}

void tranToText(tran *tx, FILE *fp, char *buf, int len)
{
    //3 tabs
    snprintf(buf, len, "\t\t{\
\n\t\t\tTtime: %d,\
\n\t\t\tTid  : %d,\
\n\t\t\tTsrc : %ld,\
\n\t\t\tTdest: %ld,\
\n\t\t\tTsum : %ld,\
\n\t\t\tTkey : %ld,\
\n\t\t},\n", tx->time, tx->id, tx->src, tx->dest, tx->amount, tx->key);

    fputs(buf, fp);
}

void blockToText(block *bx, FILE *fp, char *buf, int len)
{
    //2 tabs
    uint32_t i;
    snprintf(buf, len, "\t{\
\n\t\tBtime: %d,\
\n\t\tBcrc : %d,\
\n\t\tBpack: %d,\
\n\t\tBtran: %d,\
\n\t\tB#   : %d,\
\n\t\tBkey : %ld,\n", bx->time, bx->crc, bx->nPack, bx->nTran, bx->n, bx->key);

    fputs(buf, fp);
    for (i = 0; i < bx->nPack; i++) {packToText(bx->packs[i], fp, buf, len);}
    for (i = 0; i < bx->nTran; i++) {tranToText(bx->trans[i], fp, buf, len);}
    fputs("\t},\n", fp);
}

bool chainToText(chain *ch, FILE *fp)
{
    //1 tab
    uint32_t i;
    int len = 10000;
    char *buf = (char *)malloc(sizeof(char) * (len + 1));
    if (buf == NULL || fp == NULL)
        return 0;
    
    snprintf(buf, len, "{\n\tCtime: %d,\n\tCsize: %d,\n", ch->time, ch->size);
    
    fputs(buf, fp);
    for (i = 0; i < ch->size; i++) {blockToText(ch->head[i], fp, buf, len);}
    fputs("},\n", fp);
    
    free(buf);
    return 1;
}

pack *text2Pac(FILE *fp)
{
    /*3 tabs
    snprintf(buf, len, "\t\t{\
\n\t\t\tinfo: %s,\
\n\t\t\tdn  : %s,\
\n\t\t\txl  : %ld,\
\n\t\t\txt  : %s,\
\n\t\t\ttr  : %s,\
\n\t\t},\n", pk->info, pk->dn, pk->xl, pk->xt, pk->tr);

    fputs(buf, fp);*/
    return NULL;
}

tran *text2Tran(FILE *fp)
{
    /*3 tabs
    snprintf(buf, len, "\t\t{\
\n\t\t\ttime: %d,\
\n\t\t\tid  : %d,\
\n\t\t\tsrc : %ld,\
\n\t\t\tdest: %ld,\
\n\t\t\tsum : %ld,\
\n\t\t\tkey : %ld,\
\n\t\t},\n", tx->time, tx->id, tx->src, tx->dest, tx->amount, tx->key);

    fputs(buf, fp);*/
    return NULL;
}

block *text2Block(FILE *fp)
{
    char c;
    uint32_t nPack = 0;
    uint64_t key = 0;
    pack **packs = NULL;

    while ((c = fgetc(fp)) != EOF) {
        if (c == '{') {
            pack *px = text2Pac(fp);
            if (px != NULL) {
                nPack++;
                if (nPack > MAX_U16) {
                    deletePack(px);
                    printf("nPack limit reached\n");
                    break;
                }
                packs = (pack**)realloc(packs, sizeof(pack *) * nPack);
                packs[nPack - 1] = px;
            }
        } else if (c == 'B') {
            
        }
    }
    return newBlock(key, nPack, packs);
}

chain *text2Chainz(FILE *fp)
{
    char c;
    chain *ch = newChain();

    while (ch != NULL && (c = fgetc(fp)) != EOF) {
        if (c == '{') {
            block *bx = text2Block(fp);
            if (bx != NULL) {
                if (!insertBlock(bx, ch)) {printf("insertBlock failed");}
            }
        } else if (c == 'C') {
            
        }
    }
    return ch;
}

//! TODO AC
//! redirect the file stream or something, instead of writing to a file then telling 7z to open it
//  return 1 for success, 0 for failure
bool chainCompactor(chain *ch, char *outFile)
{
    char tmp[] = "temp.file\0";
    FILE *fp = fopen(tmp, "w");
    
    if (!chainToText(ch, fp))
    {
        printf("\n! Conversion to text failed\n");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    
    //args to compress: (ignore),        mode,     intensity,dictionary size,     #fast bytes,
    char *args[] = {(char *)"7z", (char *)"e", (char *)"-a0", (char *)"-d16", (char *)"-fb32",
    //                    input,  output,   # of threads, terminator
                    (char *)tmp, outFile, (char *)"-mt8", NULL};
    wrap7z(8, (const char **)args);

    return 1;
}

//! TODO AC
//! redirect the file stream or something, instead of writing to a file then telling 7z to open it
//  return 1 for success, 0 for failure
chain *chainExtractor(char *inFile)
{
    char tmp[] = "temp.file\0";
    //args to compress: (ignore),        mode,       input,  output,   # of threads, terminator
    char *args[] = {(char *)"7z", (char *)"d", inFile, (char *)tmp, (char *)"-mt8", NULL};
    wrap7z(5, (const char **)args);
    
    FILE *fp = fopen(tmp, "r");
    chain *ch = text2Chainz(fp);
    fclose(fp);
    
    if (ch == NULL) {printf("\n! Conversion from 7z failed\n");}
    
    return ch;
}
