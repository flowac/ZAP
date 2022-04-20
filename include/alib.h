/**
 * @file alib.h
 * @brief Functions that deal with the initialization of
 * the block structure
 */
#ifndef _ALIB_H
#define _ALIB_H

#include "atype.h"

/**
 * @brief Packs uint64_t into uint8_t buffer little endian style
 */
uint32_t u64Packer(uint8_t *buf, uint64_t data);

/**
 * @brief This function will print the relative information of a block
 * 
 * it will print the:
 *	time (when the block was created)
 *	key (Unique identifier)
 *	number of payloads
 */
void printBlock(block *target);

/**
 * @brief Create a new pack (magnet link info)
 * 
 * This fn will allocate a pack struct, and all of its parameters
 * @return True if success
 */
bool newPack(pack *px,
             char *dn,   //!< Display name
             uint64_t xl,//!< Exact length (size in bytez)
             char *xt,   //!< exact topic (URN with hash of file)
             char *tr);  //!< tracker url

void newTran(tran *tx);

bool newBlock(block *bx,
			  uint64_t n,
              uint64_t time,
              uint32_t n_packs,//package count
              pack *packs,     //package array
              uint32_t n_trans,//transaction count
              tran *trans);    //transaction array

//! return true on success
bool insertBlock(block *bx, chain *ch);

void deletePack(pack *target);

void deleteTran(tran *target);

void deleteChain(chain *target);

bool enqueuePack(pack *target);

bool enqueueTran(tran *target);

pack *dequeuePack(void);

tran *dequeueTran(void);

/**
 * @brief Validate content against internal checksums
 *
 * Chain length will be reset to the index of the first invalid block
 * @return True if no problems found
 */
bool auditChain(chain *ch);

/**
 * @brief Compare two chains
 *
 * @return The index of the first disagreement
 */
uint64_t compareChain(chain *left, chain *right);

#endif //_ALIB_H
