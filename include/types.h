/**
 * @file atype.h
 * @brief File containing some struct definitions for the coin
 *
 * The structs in this file mainly relate to the alib functions,
 * they also define the coin's structure.
 */

#ifndef _ATYPE_H
#define _ATYPE_H

#include <queue>
#include <vector>
#include <stdint.h>

#define LOG 0                 //!< not sure

#define MAX_U2   0x03U
#define MAX_U4   0x0FU
#define MAX_U6   0x3FU
#define MAX_U8   0xFFUL
#define MAX_U16  0xFFFFUL
#define MAX_U30  0x3FFFFFFFUL
#define MAX_U32  0xFFFFFFFFUL
#define MAX_U34  0x3FFFFFFFFUL
#define MAX_U64  0xFFFFFFFFFFFFFFFFULL

#define ONE_BILLION 1000000000

#define B_MAX    5000         //!< max number of blocks
#define FRAC_MAX 10000        //!< max size of floating point balance
#define BUF64    0x40U
#define BUF1K    0x400UL
#define BUF4K    0x1000UL

#define ED448_LEN       57    //!< number of bytes for an ED448 key
#define ED448_SIG_LEN   (ED448_LEN * 2)
#define SHA3_LEN        48    //!< number of bytes for a SHA3-384
#define SHAKE_LEN       16    //!< number of bytes for a SHAKE-128
#define MAGNET_XT_LEN   20    //!< 160 bit file checksum
#define MAGNET_KT_COUNT 5     //!< max number of search keywords
#define MAGNET_KT_LEN   16    //!< longest possible search keyword
#define MAGNET_DN_LEN   128   //!< must not exceed MAX_U8
#define MAGNET_TR_LEN   1024  //!< must not exceed MAX_U16

/**
 * @brief Holds information about the parameters of the magnet link
 */
typedef struct {
	uint8_t  crc[SHAKE_LEN];    //!< checksum of all members below
	uint8_t  xt[MAGNET_XT_LEN]; //!< exact topic, 160 bit file hash
	uint64_t xl; //!< exact length, size of file in bytes
	char    *dn; //!< display name, filename
	uint8_t *tr; //!< address tracker, tracker url
	char    *kt[MAGNET_KT_COUNT]; //!< search keywords, upto MAGNET_KT_LEN each
} pack;

/**
 * @brief Holds information about a transaction
 */
typedef struct {
	uint64_t id;
	uint64_t deci; //Integer value
	uint16_t frac; //Floating point value, limit of FRAC_MAX
	uint8_t  src[ED448_LEN];
	uint8_t  dest[ED448_LEN];
	uint8_t  sig[ED448_SIG_LEN];
} tran;

/**
 * @brief Holds information about a block
 */
typedef struct {
	uint8_t  crc[SHA3_LEN]; //!< checksum bits
	uint8_t  key[SHA3_LEN]; //!< gen next bits
	uint64_t n;      //!< block number
	uint64_t time;   //!< epoch seconds
	uint8_t  n_trans;//!< number of transactions, 255 per block max
	tran *trans;
} block;

typedef struct {
	uint8_t address[SHA3_LEN]; //!< SHA3 checksum
	//If balance is less than 1, the entry shall be trimmed
	uint64_t deci; //Integer value
	uint16_t frac; //Floating point value, limit of FRAC_MAX
} balance;

typedef struct {
	std::vector<balance> bal;
	std::vector<block> blk;
	std::vector<pack> pak;
} chain;

#endif //_ATYPE_H
