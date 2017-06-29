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
    if (!pk || !fp || !buf) return;
    snprintf(buf, len, "\t{P\
\n\t\tPinfo: %s,\
\n\t\tPdn  : %s,\
\n\t\tPlen : %lld,\
\n\t\tPxt  : %s,\
\n\t\tPtr  : %s,\
\n\tP},\n", pk->info, pk->dn, pk->xl, pk->xt, pk->tr);

    fwrite(buf, 1, strlen(buf), fp);
}

void tranToText(tran *tx, FILE *fp, char *buf, int len)
{
    //2 tabs
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
    
    strcpy(buf, "B},\n");
    fwrite(buf, 1, strlen(buf), fp);
}

void *chainToText(void *args)
{
    threadParams *tp = (threadParams *)args;
    uint8_t part =      tp->i;
    block **head =      tp->head;
    uint32_t start =    tp->start;
    uint32_t target =   tp->end;
    //1 tab
    char tmp[16];
    snprintf(tmp, 15, "temp%u.file", part);
    FILE *fp = fopen(tmp, "w");
    
    uint32_t i;
    int len = 3000;

    char *buf = (char *)malloc(sizeof(char) * (len + 1));
    // char buf[3001];
    if (buf == NULL || fp == NULL) {
        char msg[80];
        snprintf(msg, 79, "\nCreating part [%u] failed, name [%s], pointer [%p][%d]\n",
                part, tmp, fp, fp);
        log_msg_custom(msg);
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
    
    //compress_file(tmp);
    
    return NULL;
}

void *chainToText_to_file(chain *ch, uint8_t parts)
{
    uint8_t part =      parts;
    block **head =      ch->head;
    uint32_t start =    0;
    uint32_t target =   ch->size;
    //1 tab
    char tmp[16];
    snprintf(tmp, 15, "orig%u.file", part);
    FILE *fp = fopen(tmp, "w");
    
    uint32_t i;
    int len = 3000;

    char *buf = (char *)malloc(sizeof(char) * (len + 1));
    if (buf == NULL || fp == NULL) {
        char msg[80];
        snprintf(msg, 79, "\nCreating part [%u] failed, name [%s], pointer [%p][%d]\n",
                part, tmp, fp, fp);
        log_msg_custom(msg);
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
    
    return NULL;
}

char *indexes_of(char *haystack, const char *needle_start,
                 const char *needle_end)
{
    char *ptr_start = strstr(haystack, needle_start) + 2;
    char *ptr_end = strstr(haystack, needle_end);
    int len = ptr_end - ptr_start;
    char *dest = (char *)malloc(sizeof(char)* len + 1); 
    dest[len] = '\0';
    memcpy(dest, ptr_start, len);
    return dest;
}

pack *text2Pac(FILE *fp)
{
    char s[MAX_U8 + 1],
        *dn = NULL,
        *xt = NULL,
        *p_xl = NULL,
        *tr = NULL;
    uint64_t xl;

    pack *px = NULL;

    while (fgets(s, MAX_U8, fp) != NULL) {
        int len = 0;
        char *data = strstr(s, (char *)"P");
        if (data){
            len = strlen(data);
        }
        if (len > 1) {
            switch (data[1]) {
            case 'i': // pack->info
                break;
            case 'd': // pack->dn
                dn = indexes_of(data, ": ", ",");
                break;
            case 'l': // pack->xl
                p_xl = indexes_of(data, ": ", ",");
                xl = atof(p_xl);
                break;
            case 'x': // pack->xt
                xt = indexes_of(data, ": ", ",");
                break;
            case 't': // pack->tr
                tr = indexes_of(data, ": ", ",");
                break;
            case '}': // end of pack
                px = newPack(dn, xl, xt, tr);
                if (dn)
                    free(dn);
                if (p_xl)
                    free(p_xl);
                if (xt)
                    free(xt);
                if (tr)
                    free(tr);
                return px;
            default :
                break;
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
    pack **packs = NULL;
    char * tmp;
    uint32_t time = 0;
    uint32_t crc = 0;
    uint16_t n_pack = 0;
    uint16_t n_tran = 0;
    uint32_t n = 0;
    uint64_t key = 0;
    block *new_block = NULL;


    while (fgets(s, MAX_U8, fp) != NULL) {
        if (strstr(s, (char *)"{P") != NULL) {
            pack *px = text2Pac(fp);
            if (px != NULL) {
                n_pack++;
                if (n_pack > MAX_U16) {
                    deletePack(px);
                    log_msg_custom("nPack limit reached\n");
                    break;
                }
                packs = (pack**)realloc(packs, sizeof(pack *) * n_pack);
                packs[n_pack - 1] = px;
            }
        } else {
            int len = 0;
            char *data = strstr(s, (char *)"B");
            if (data)
                len = strlen(data);
            if (len > 1) {
                switch (data[1]) {
                case 'g': // block->time
                    tmp = indexes_of(data, ": ", ",");
                    time = atol(tmp);
                    free(tmp);
                    break;
                case 'c': // block->crc
                    tmp = indexes_of(data, ": ", ",");
                    crc = atol(tmp);
                    free(tmp);
                    break;
                case 'p': // block->nPack
                    /* n_pack is being counted in the other
                     * part of the loop
                     */
                    //tmp = indexes_of(data, ": ", ",");
                    //n_pack = atol(tmp);
                    //free(tmp);
                    break;
                case 't': // block->nTran
                    tmp = indexes_of(data, ": ", ",");
                    n_tran = atol(tmp);
                    free(tmp);
                    break;
                case 'n': // block->n
                    tmp = indexes_of(data, ": ", ",");
                    n = atol(tmp);
                    free(tmp);
                    break;
                case 'k': // block->key
                    tmp = indexes_of(data, ": ", ",");
                    key = strtoll(tmp, NULL, 10);
                    free(tmp);
                    break;
                case '}':
                    new_block = restore_block(time, crc, n_pack,
                                                  n_tran, n, key,
                                                  packs);
                    return new_block;
                    break;
                default :
                    break;
                }
            }
        }
    }
    //return (block *)newBlock(0, key, n_pack, packs);
    return new_block;
}

int text2Chainz(FILE *fp, chain *ch)
{
    char s[MAX_U8 + 1];
    if (ch == NULL)
        return 0;
    
    /* Parse the text searching for the start of a block
     *
     */
    while (fgets(s, MAX_U8, fp) != NULL) {
        if (strstr(s, (char *)"{B") != NULL) {
            if (!insertBlock(text2Block(fp), ch))
                log_msg_custom("Failed to insert block");
        } else {
            int len = 0;
            char *data = strstr(s, (char *)"C");
            if (data)
                len = strlen(data);
            if (len > 1) {
                switch (data[1]) {
                    case 't':
                        // Ctime
                        break;
                        // Csize
                    case 's':
                        break;
                    default :
                        break;
                }
            }
        }
    }
    return 1;
}

chain *file_2_chainz(FILE *fp)
{
    chain *ch = newChain();
    if(!text2Chainz(fp, ch))
        log_msg_default;
    return NULL;
}

//  return 1 for success, 0 for failure
bool chainCompactor(chain *ch, uint8_t parts)
{
    uint32_t size = ch->size,
        target, // # of blocks each thread will compresss
        done; // # of blocks assigned to threads
    uint8_t i;
    
    if (parts == 0 || parts > MAX_U8) {
        parts = 1;
    }
    pthread_t threads[parts];
    threadParams tp[parts];
    
    done = 0;
    target = size / parts;
    
    for (i = 0; i < parts; i++) {
        tp[i].i = i + 1;
        tp[i].head = ch->head;
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

//  return 1 for success, 0 for failure
/* Right now dont worry about compressing/decompressing
 */
chain *chain_extractor(const char *inFile, uint8_t parts)
{
    char buffy[69];
    chain *ch = newChain();
    FILE *fp = NULL;
    for (int i = 1; i <= parts; i++){
        sprintf(buffy,"temp%d.file",i);
        fp = fopen(buffy, "r");
        if (fp == NULL){
            log_msg_default;
            continue;
        }
        text2Chainz(fp, ch);
    }
    return ch;
}
