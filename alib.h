#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "boost/date_time/posix_time/posix_time.hpp"

/*typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;
typedef unsigned long int uint64_t;*/

#define LOG      0
#define MAX_ZIP  255//size of each
#define MAX_PACK 255//number of

namespace pt = boost::posix_time;

typedef struct
{
    char info[4];//first 4 characters of name
    uint16_t crc;//checksum
    uint8_t len;//length of data in bytes
    uint8_t pad;
    uint8_t *zip;//compressed data
}pack;

typedef struct
{
    uint32_t time;//epoch seconds
    uint16_t crc;//checksum
    uint8_t  nPack;//number of payloads, 255 per block max
    uint8_t  pad;
    uint32_t trans;//number of transactions
    uint32_t n;//block number
    uint64_t key;//gen next
    pack *packs;//variable size
}block;

typedef struct
{
    uint32_t time;//time of last update
    uint32_t size;//4 billion should be more than enough
    block **head;//expandable
}chain;

typedef struct
{
    char *url;
    char *name;
    char *desc;
}meta;

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

block *newBlock(uint64_t key, uint32_t nPack, pack *packs)
{
    block *bx = (block *)malloc(sizeof(block));

    bx->time = (uint32_t)sNow();
    bx->key = key;
    bx->nPack = nPack < MAX_PACK ? nPack : MAX_PACK;
    bx->packs = packs;//! Don't free the payload pointer

    if (!LOG) return bx;

    printTime(sNow());
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

uint32_t deleteBlock(block *target)
{
    free(target->packs);
    return target->nPack * sizeof(pack);
}

uint32_t deleteChain(chain *target)
{
    uint32_t bytes = 0;
    for (uint32_t i = 0; i < target->size; i++)
    {
        bytes += deleteBlock(target->head[i]);
        free(target->head[i]);
        bytes += sizeof(block);
    }
    free(target->head);
    return bytes;

    //! And then free the chain pointer from the caller
}
