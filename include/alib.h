/**
 * @file alib.h
 * @brief Functions that deal with the initialization of
 * the block structure
 *
 * Essentially just cleanup/init functions
 */
#ifndef _ALIB_H
#define _ALIB_H


//#include <boost/date_time/posix_time/posix_time.hpp>

#include "atype.h"
#include <time.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

//namespace pt = boost::posix_time;

/**
 * @brief Get current time
 *
 * Get the current time in seconds since Epoch
 */
inline time_t sNow();

block *restore_block(uint32_t     time,
                         uint32_t crc,
                         uint16_t n_pack,
                         uint16_t n_tran,
                         uint32_t n,
                         uint64_t key,
                         pack **packs
                         );

/**
 * @brief Print current time
 */
inline void printTime(time_t time //!< The time to be printed.
);

/**
 * @brief This function will print the relative information of a block
 * 
 * it will print the:
 *	time (when the block was created)
 *	key (Unique identifier)
 *	number of payloads
 */
void printBlock(block *target //!< The block to be printed
);

/**
 * @brief Create a new pack (magnet link info)
 * 
 * This fn will allocate a pack struct, and all of its parameters
 * @return NULL - something went wrong :( (malloc failed) \n
 * ptr to new pack struct
 */
pack *newPack(char         *dn, //!< Display name
                  uint64_t  xl, //!< Exact length (size in bytez)
                  char     *xt, //!< exact topic (URN with hash of file)
                  char *tr      //!< tracker url
                  );

tran *newTran();

/**
 * @brief Create a new block
 * 
 */
block *newBlock(uint32_t n,
                    uint64_t key,
                    uint32_t nPack,
                    pack **packs
                    );

chain *newChain(void);

//! return 1 on success
bool insertBlock(block *bx,
                     chain *ch
                     );

void deletePack(pack *target
);

void deleteChain(chain *target
);

#endif//_ALIB_H

