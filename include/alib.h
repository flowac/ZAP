#ifndef _ALIB_H
#define _ALIB_H


//#include <boost/date_time/posix_time/posix_time.hpp>

#include "atype.h"
#include <time.h>
#include <stdio.h>

//namespace pt = boost::posix_time;

/* Get current time
 * DESCRIPTION:
 * Get the current time in seconds since Epoch
 */
inline time_t sNow();

inline void printTime(time_t time);

/* This function will print the relative information of a block
 * DESCRIPTION:
 * it will print the:
 *	time (when the block was created)
 *	key (whatever that mean @alien)
 *	number of payloads
 */
void printBlock(block *target);

/* Create a new pack (magnet link info)
 * DESCRIPTION:
 * it will allocate a pack struct, and all of its parameters
 * INPUT:
 * char * dn - display name
 * uint64_t xl - exact length (size in bytez)
 * char * xt - exact topic (URN with hash of file)
 * char * tr - tracker url
 * RETURN:
 * NULL - something went wrong :( (malloc failed)
 * ptr to new pack struct
 */
pack *newPack(char *dn, uint64_t xl, char *xt, char *tr);

tran *newTran();

/* Create a new block
 * DESCRIPTION:
 * 
 */
block *newBlock(uint32_t n, uint64_t key, uint32_t nPack, pack **packs);

chain *newChain(void);

//! return 1 on success
bool insertBlock(block *bx, chain *ch);

uint32_t deleteChain(chain *target);

#endif//_ALIB_H

