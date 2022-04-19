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

#define LOG 0                 //!< not sure

#define MAX_U2   0x03U        //!< max size of a  2 bit int
#define MAX_U4   0x0FU        //!< max size of a  4 bit int
#define MAX_U6   0x3FU        //!< max size of a  6 bit int
#define MAX_U8   0xFFUL       //!< max size of an 8 bit int
#define MAX_U16  0xFFFFUL     //!< max size of a 16 bit int
#define MAX_U32  0xFFFFFFFFUL //!< max size of a 32 bit int

#define INFO_LEN 5            //!< length of info of pack
#define B_MAX    5000         //!< max number of blocks

#define BUF4K    0x1000UL

/**
 * @brief not sure
 */
enum Link {
	P_DN = 0,//!< display name
	P_XL,    //!< exact length
	P_XT,    //!< exact topic
	P_TR,    //!< address tracker
	P_LEN    //!< number of total parameters, must be last
};

/**
 * @brief Holds information about the parameters of the magnet link
 */
typedef struct {
	uint64_t xl; //!< exact length, size of file in bytez
	char *dn;    //!< display name, filename
	char *xt;    //!< exact topic, URN with hash of file
	char *tr;    //!< address tracker, tracker url
	char info[INFO_LEN + 1];
} pack;

/**
 * @brief Holds information about a transaction
 */
typedef struct {
	uint64_t time;
	uint64_t id;
	uint64_t amount;
	// TODO: change all of the below into 256 bit uint8_t
	uint64_t src;
	uint64_t dest;
} tran;

/**
 * @brief Holds information about a block
 */
typedef struct {
	uint8_t *key;    //!< gen next 512 bits
	uint8_t *crc;    //!< checksum 512 bits
	uint64_t n;      //!< block number
	uint64_t time;   //!< epoch seconds
	uint8_t  n_packs;//!< number of payloads, 255 per block max
	uint8_t  n_trans;//!< number of transactions, 255 per block max
	pack *packs;
	tran *trans;
} block;

typedef struct {
	uint8_t address[64]; //!< SHA3-512 checksum
	//If balance is less than 1, the entry shall be trimmed
	uint64_t id;         //SHAKE-128(64) of SHA3-512 checksum (for sorting)
	uint64_t deci;       //Decimal value
	uint64_t frac;       //Floating point value
} balance;

typedef struct {
	uint64_t n_bal;
	uint64_t n_blk;
	block blk[B_MAX];
	balance *bal;
} chain;

#endif //_ATYPE_H
