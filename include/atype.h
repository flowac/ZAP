#ifndef _ATYPE_H
#define _ATYPE_H

//#include <boost/cstdint.hpp>
#include <stdint.h>

#define LOG     0
#define MAX_U8  255//max size of unsigned 8 bit int
#define MAX_U16 65535
#define MAX_U32 4294967295

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

/* This struct will hold the information on the parameters of
 * the magnet link
 * PARAMETERS:
 * info - 
 * dn - (display name) i.e. file name
 * xl - (exact length) size of file in bytez
 * xt - (exact topic) URN with the hash of the file
 * tr - tracker url, google "tracker url" for more info
 */
typedef struct
{
    char info[6];//first 5 characters of name, null terminated
    char *dn; // display name
    uint64_t xl; // exact length
    char *xt; // exact topic
    char *tr; // address tracker
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

/* The blockchain structure
 * PARAMETERS:
 * uint32_t time - time of the last update
 * uint32_t size - number of blocks
 * block **head - dynamic array of blocks
 */
typedef struct
{
    uint32_t time;//time of last update
    uint32_t size;//4 billion should be more than enough
    block **head;//expandable
}chain;

typedef struct{
    uint32_t i;
    block **head;
    uint32_t start;
    uint32_t end;
}threadParams;

#endif//_ATYPE_H
