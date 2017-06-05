#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "alib.h"
#include "log.h"
#include "alibio.h"
#include "lzma_wrapper.h"
#include "C/LzmaEnc.h"


void packToText(pack *pk, FILE *fp, char *buf, int len)
{
    //2 tabs
    /* @FLOWING WATER WHAT THE FUCK U SAID I HAD TO COMPARE WITH NULL WHAT THE FUCK */
    if (!pk || !fp || !buf) return;
    snprintf(buf, len, "\t{P\
\n\t\tPinfo: %s,\
\n\t\tPdn  : %s,\
\n\t\tPlen : %ld,\
\n\t\tPxt  : %s,\
\n\t\tPtr  : %s,\
\n\t},\n", pk->info, pk->dn, pk->xl, pk->xt, pk->tr);

    fwrite(buf, 1, strlen(buf), fp);
}

void tranToText(tran *tx, FILE *fp, char *buf, int len)
{
    //2 tabs
    /* @FLOWING WATER WHAT THE FUCK U SAID I HAD TO COMPARE WITH NULL WHAT THE FUCK */
    if (!tx || !fp || !buf)
        return;
    snprintf(buf, len, "\t{T\
\n\t\tTtime: %d,\
\n\t\tTid  : %d,\
\n\t\tTsrc : %ld,\
\n\t\tTdest: %ld,\
\n\t\tTsum : %ld,\
\n\t\tTkey : %ld,\
\n\t},\n", tx->time, tx->id, tx->src, tx->dest, tx->amount, tx->key);

    fwrite(buf, 1, strlen(buf), fp);
}

void blockToText(block *bx, FILE *fp, char *buf, int len)
{
    //1 tabs
    uint32_t i;
    snprintf(buf, len, "{B\
\n\tBgmt : %d,\
\n\tBcrc : %d,\
\n\tBpack: %d,\
\n\tBtran: %d,\
\n\tBn   : %d,\
\n\tBkey : %ld,\n", bx->time, bx->crc, bx->nPack, bx->nTran, bx->n, bx->key);

    fwrite(buf, 1, strlen(buf), fp);
    
    for (i = 0; i < bx->nPack; i++) {
        packToText(bx->packs[i], fp, buf, len);
    }
    for (i = 0; i < bx->nTran; i++) {
        tranToText(bx->trans[i], fp, buf, len);
    }
    
    strcpy(buf, "},\n");
    fwrite(buf, 1, strlen(buf), fp);
}

void *chainToText(void *args)//uint8_t part, block **head, uint32_t start, uint32_t target)
{
    threadParams *tp = (threadParams *)args;
    uint8_t part =      tp->i;
    block **head =      tp->head;
    uint32_t start =    tp->start;
    uint32_t target =   tp->end;
    //1 tab
    char tmp[16], tmp7z[16];
    snprintf(tmp, 15, "temp%u.file", part);
    snprintf(tmp7z, 15, "temp%u.7z", part);
    FILE *fp = fopen(tmp, "w");
    
    uint32_t i;
    int len = 3000;
    /* @flowingwater, why are we mallocing this? just declare buff
     * as a char array...
     */
    char *buf = (char *)malloc(sizeof(char) * (len + 1));
    if (buf == NULL || fp == NULL) {
        log_msg_default;
        return NULL;
    }
    
    snprintf(buf, len, "Ctime: %u,\nCsize: %u,\n", part, target);
    
    fwrite(buf, 1, strlen(buf), fp);
    
    for (i = start; i < target; i++) {
        blockToText(head[i], fp, buf, len);
    }
    
    strcpy(buf, "EOF\n");
    fwrite(buf, 1, strlen(buf), fp);
    
    fclose(fp);
    free(buf);
    
    compress_file(tmp);
    
    return NULL;
}

pack *text2Pac(FILE *fp)
{
    char s[MAX_U8 + 1];
    pack *px = NULL;

    while (fgets(s, MAX_U8, fp) != NULL) {
        char *data = strstr(s, (char *)"P");
        int len = strlen(data);
        if (len > 1) {
            switch (data[1]) {
                case 'i':   break;
                case 'd':   break;
                case 'l':   break;
                case 'x':   break;
                case 't':   break;
                default :   break;
            }
        }
    }
    return px;
}

tran *text2Tran(FILE *fp)
{
    return NULL;
}

block *text2Block(FILE *fp)
{
    char s[MAX_U8 + 1];
    uint32_t nPack = 0;
    uint64_t key = 0;
    pack **packs = NULL;

    while (fgets(s, MAX_U8, fp) != NULL) {
        if (strstr(s, (char *)"{P") != NULL) {
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
        } else {
            char *data = strstr(s, (char *)"B");
            int len = strlen(data);
            if (len > 1) {
                switch (data[1]) {
                    case 'g':   break;
                    case 'c':   break;
                    case 'p':   break;
                    case 't':   break;
                    case 'n':   break;
                    case 'k':   break;
                    default :   break;
                }
            }
        }
    }
    return (block *)newBlock(0, key, nPack, packs);
}

chain *text2Chainz(FILE *fp)
{
    char s[MAX_U8 + 1];
    chain *ch = newChain();
    if (ch == NULL) return NULL;
    
    while (fgets(s, MAX_U8, fp) != NULL) {
        if (strstr(s, (char *)"{B") != NULL) {
            if (!insertBlock(text2Block(fp), ch))   printf("insertBlock failed");
        } else {
            char *data = strstr(s, (char *)"C");
            int len = strlen(data);
            if (len > 1) {
                switch (data[1]) {
                    case 't':   break;
                    case 's':   break;
                    default :   break;
                }
            }
        }
    }
    return ch;
}

//! TODO AC
//! redirect the file stream or something, instead of writing to a file then telling 7z to open it
//  return 1 for success, 0 for failure
bool chainCompactor(chain *ch, uint8_t parts)
{
    uint32_t size = ch->size, target, done;
    uint8_t i;
    
    if (parts == 0 || parts > MAX_U8) {
        parts = 1;
    }
    pthread_t threads[parts];
    threadParams tp[parts];
    
    done = 0;
    target = size / parts;
    
    for (i = 0; i < parts; i++) {
        tp[i].i = i + 1; /* why not just zero index smh */
        tp[i].head = ch->head; /* why is this in the loop? */
        tp[i].start = done;
        done += target;
        if (i == 0)
            done += (size % parts);
        tp[i].end = done;
        
        pthread_create(&threads[i], NULL, &chainToText, (void *)&tp[i]);
    }
    for (i = 0; i < parts; i++)
        pthread_join(threads[i], NULL);

    return 1;
}

//! TODO AC
//! redirect the file stream or something, instead of writing to a file then telling 7z to open it
//  return 1 for success, 0 for failure
chain *chainExtractor(char *inFile)
{
    char tmp[] = "temp.file\0";
    compress_file(tmp);
    
    FILE *fp = fopen(tmp, "r");
    chain *ch = text2Chainz(fp);
    fclose(fp);
    
    if (ch == NULL) {printf("\n! Conversion from 7z failed\n");}
    
    return ch;
}
