#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "boost/date_time/posix_time/posix_time.hpp"

/*typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;
typedef unsigned long int uint64_t;*/

#define LOGLVL 2
#define MAXLEN 8*1024*1024//8MB

#define LMIN   1
#define LMAX   2

typedef struct
{
    uint32_t time;//epoch seconds
    uint64_t key;//gen next
    uint32_t len;//length of payload
    uint64_t *payload;//variable size
}block;

typedef struct
{
    uint32_t size;
    block **head;//expandable
}chain;

typedef struct
{
    char *url;
    char *name;
    char *desc;
    uint64_t *data;
}p7z;

void printTime(time_t time)
{
    namespace pt = boost::posix_time;
    pt::ptime tout = pt::from_time_t(time);
    std::cout << tout << std::endl;
}

void printBlock(block *target)
{
    uint32_t len = target->len;

    printTime((time_t) target->time);
    printf("[%u] > 0x%lX [%u]\n",
           target->time, target->key, target->len);

    if (LOGLVL < LMAX) return;
    for (uint32_t i = 0; i < len; i++)
    {
        printf("%lX\t", target->payload[i]);
    }
    printf("\n");
}

block *newBlock(uint64_t key, uint32_t len, uint64_t *payload)
{
    block *bx = (block *)malloc(sizeof(block));

    time_t rawT;
    time(&rawT);
    printTime(rawT);

    bx->time = (uint32_t)rawT;
    bx->key = key;
    bx->len = len < MAXLEN ? len : MAXLEN;
    bx->payload = payload;//! Don't free the payload pointer
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
    return 1;
}

void deleteBlock(block *target)
{
    free(target->payload);
}

void deleteChain(chain *target)
{
    for (uint32_t i = 0; i < target->size; i++)
    {
        deleteBlock(target->head[i]);
        free(target->head[i]);
    }
    free(target->head);
    //! And then free the chain pointer from the caller
}
