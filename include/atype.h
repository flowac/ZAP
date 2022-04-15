/**
 * @file atype.h
 * @brief File containing some struct definitions for the coin
 *
 * The structs in this file mainly relate to the alib functions,
 * they also define the coin's structure.
 */

#ifndef _ATYPE_H
#define _ATYPE_H

#include <stdint.h>

#define LOG 0                //!< not sure

#define MAX_U6  0x3FU        //!< max size of a  6 bit int
#define MAX_U8  0xFFU        //!< max size of an 8 bit int
#define MAX_U16 0xFFFFU      //!< max size of a 16 bit int
#define MAX_U32 0xFFFFFFFFUL //!< max size of a 32 bit int
#define B_MAX   5000         //!< max number of blocks

/**
 * @brief not sure
 */
enum Link {
	DN = 0,//!< display name
	XL,    //!< exact length
	XT,    //!< exact topic
	TR,    //!< address tracker
	MLEN   //!< number of total parameters, must be last
};

/**
 * @brief Holds information about the parameters of the magnet link
 */
typedef struct {
	char info[6];//!< first 5 characters of name, null terminated
	char *dn;    //!< display name, filename
	uint64_t xl; //!< exact length, size of file in bytez
	char *xt;    //!< exact topic, URN with hash of file
	char *tr;    //!< address tracker, tracker url
} pack;

/**
 * @brief Holds information about a transaction
 */
typedef struct {
	uint32_t time;
	uint32_t id;
	uint64_t src;
	uint64_t dest;
	uint64_t amount;
	uint64_t key;
} tran;

/**
 * @brief Holds information about a block
 */
typedef struct {
	uint32_t time;   //!< epoch seconds
	uint64_t crc;    //!< checksum
	uint64_t n_packs;//!< number of payloads, 255 per block max
	uint64_t n_trans;//!< number of transactions
	uint64_t n;      //!< block number
	uint64_t key;    //!< gen next
	pack *packs;
	tran *trans;
} block;

typedef struct {
	uint8_t address[512];//!< SHA3-512 checksum
	//If balance is less than 1, the entry shall be trimmed
	uint64_t id;         //SHAKE-128(64) of SHA3-512 checksum (for sorting)
	uint64_t deci;       //Decimal value
	uint64_t frac;       //Floating point value
} balance;

typedef struct {
	uint64_t n_bal;
	uint64_t n_blk;
	balance *bal;
	block blk[B_MAX];
} chain;

#endif //_ATYPE_H
