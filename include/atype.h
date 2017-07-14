/**
 * @file atype.h
 * @brief File containing some struct definitions for the coin
 *
 * The structs in this file mainly relate to the alib functions,
 * they also define the coin's structure.
 */

#ifndef _ATYPE_H
#define _ATYPE_H

                     //#include <boost/cstdint.hpp>
#include <stdint.h>

#define LOG 0                   //!< not sure

#define MAX_U8   255U           //!< max size of an u8 bit int
#define MAX_U16  65535U         //!< max size of u16 int
#define MAX_U32  4294967295UL   //!< maz size of a u32 int

/*
 * typedef unsigned char     uint8_t;
 * typedef unsigned short    uint16_t;
 * typedef unsigned int      uint32_t;
 * typedef unsigned long int uint64_t;*/

/**
 * @brief not sure
 */
enum Link
{
    DN = 0,          //!< display name
    XL,              //!< exact length
    XT,              //!< exact topic
    TR,              //!< address tracker
    MLEN             //!< number of total parameters, must be last
};

/**
 * @brief Holds information about the parameters of the magnet link
 */
typedef struct
{
    char info[6];    //!< first 5 characters of name, null terminated
    char *dn;        //!< display name, filename
    uint64_t xl;     //!< exact length, size of file in bytez
    char *xt;        //!< exact topic, URN with hash of file
    char *tr;        //!< address tracker, tracker url
}pack;

/**
 * @brief Holds information about a transaction
 */
typedef struct
{
    uint32_t time;
    uint32_t id;
    uint64_t src;
    uint64_t dest;
    uint64_t amount;
    uint64_t key;
}tran;

/**
 * @brief Holds information about a block
 */
typedef struct
{
    uint32_t time;  //!< epoch seconds
    uint32_t crc;   //!< checksum
    uint16_t nPack; //!< number of payloads, 255 per block max
    uint16_t nTran; //!< number of transactions
    uint32_t n;     //!< block number
    uint64_t key;   //!< gen next
    pack **packs;   //!< variable size
    tran **trans;
}block;

/**
 * @brief The blockchain structure
 */
typedef struct
{
    uint32_t time;  //!< time of last update
    uint32_t size;  //!< Number of blocks, 4 bil max
    block **head;   //!< expandable, dynamic array of blocks
}chain;

/**
 * @brief Struct holding values for pthread fn call
 */
typedef struct{
    uint32_t   i;               //!<  current block num
    block    **head;            //!<  head of the block chain
    uint32_t   start;           //!<  starting block num 
    uint32_t   end;             //!<  ending block num 
}threadParams;

#endif//_ATYPE_H
