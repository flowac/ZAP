#ifndef _ATYPE_H
#define _ATYPE_H

//#include <boost/cstdint.hpp>
#include <stdint.h>

#define LOG     0
#define MAX_U8  255//max size of unsigned 8 bit int
#define MAX_U16 65535

/*typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;
typedef unsigned long int uint64_t;*/

enum Link
{
    DN = 0,//display name
    XL,    //exact length
    XT,    //exact topic
    TR,    //address tracker
    MLEN//number of total parameters, must be last
};

typedef struct
{
    char info[6];//first 5 characters of name, null terminated
    char *dn;//magnet link parameters
    uint64_t xl;
    char *xt;
    char *tr;
}pack;

typedef struct
{
    uint32_t time;
    uint32_t id;
    uint64_t src;
    uint64_t dest;
    uint64_t amount;
    uint64_t key;
}tran;

typedef struct
{
    uint32_t time;//epoch seconds
    uint32_t crc;//checksum
    uint16_t nPack;//number of payloads, 255 per block max
    uint16_t nTran;//number of transactions
    uint32_t n;//block number
    uint64_t key;//gen next
    pack **packs;//variable size
    tran **trans;
}block;

typedef struct
{
    uint32_t time;//time of last update
    uint32_t size;//4 billion should be more than enough
    block **head;//expandable
}chain;

#endif//_ATYPE_H
