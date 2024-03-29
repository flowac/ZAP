/**
 * @file main_lib.h
 * @brief Functions that deal with the initialization of
 * the block structure
 */
#ifndef _ALIB_H
#define _ALIB_H

#include <cstddef>

#include "types.h"
#include "log.h"

void filterLine(char *b, int *k);
void stemWord(char *buf, int *len); //!< PortStemmer
bool encodeMsg(const char *msg, uint32_t st[MAGNET_ST_LEN], char *&ut, uint8_t kt[MAGNET_KT_LEN]); //!< Encode a message with dictionary

/**
 * @brief Packs uint??_t into uint8_t buffer little endian style
 */
uint32_t u16Packer(uint8_t *buf, uint16_t data);
uint32_t u16Unpack(uint8_t *buf, uint16_t &data);
uint32_t u64Packer(uint8_t *buf, uint64_t data);
uint32_t u64Unpack(uint8_t *buf, uint64_t &data);
uint32_t u8len(uint8_t *ptr);
uint32_t u8cmp(uint8_t *ptr, char *str);

void printTorCat(torDB &target);
void printTorWordMap(torDB &target);

/**
 * @brief Create a new pack (magnet link info)
 * 
 * This fn will allocate a pack struct, and all of its parameters
 * @return True if success
 */
bool newPack(torDB &td,
			 uint8_t xt[MAGNET_XT_LEN], //!< Exact topic (file hash)
             uint64_t xl,               //!< Exact length (size in bytes)
             char *dn,                  //!< Display name
             uint8_t *tr,               //!< Tracker url
			 uint8_t kt,                //!< Keywords
			 uint8_t crc[SHAKE_LEN] = NULL);

bool newTran(tran *tx,
			 uint64_t id,
			 uint64_t deci,
			 uint16_t frac,
			 uint8_t src[ED448_LEN],
			 uint8_t dest[ED448_LEN],
			 uint8_t sig[ED448_SIG_LEN]);

/**
 * @brief THIS IS NOT THREAD SAFE.
 */
bool newBlock(chain &ch);

bool insertBlock(chain &ch,
				 uint64_t n,
				 uint64_t time,
				 uint32_t n_trans,//transaction count
				 tran *trans,     //transaction array
				 uint8_t crc[SHA3_LEN] = NULL,
				 uint8_t key[SHA3_LEN] = NULL);

void convertKeyword(uint8_t kt, uint8_t *kt1, uint8_t *kt2);
const char *getKeyword1(uint8_t kt1);
const char *getKeyword2(uint8_t kt1, uint8_t kt2);
bool isKeywordValid(uint8_t kt1, uint8_t kt2);
uint8_t lookupKeyword(const char *kt1, const char *kt2);
std::vector<uint32_t> searchTorDB(torDB &td, uint8_t kt1, uint8_t kt2, const char *str);
std::vector<uint32_t> searchTorDB(torDB &td, const char *kt1p, const char *kt2p, const char *str);

void deleteChain(chain &target);

/**
 * @brief: Compress tracker links in place
 *         Original buffer can be modified even if compress failed
 * @return: Length of null terminated string
 */
uint32_t compressTracker(uint8_t *tr);
/**
 * @brief: Decompress tracker links
 * @return: Length of null terminated returned string
 */
uint32_t decompressTracker(uint8_t *tr, char ret[MAGNET_TR_LEN]);

/**
 * @brief Generate or validate a checksum
 */
bool checkPack(pack &px, bool modify);
bool checkBlock(block &bx, bool modify, uint8_t crc[SHA3_LEN]);

/**
 * @brief Validate content against internal checksums
 */
bool auditTorDB(torDB &td);
bool auditChain(chain &ch);

/**
 * @brief Compare two chains
 *
 * @return The index of the first disagreement
 */
uint64_t compareChain(chain &left, chain &right);

#endif //_ALIB_H
