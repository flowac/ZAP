#ifndef _ALIB_H
#define _ALIB_H


//#include <boost/date_time/posix_time/posix_time.hpp>

#include "atype.h"
#include <time.h>

//namespace pt = boost::posix_time;

inline time_t sNow();

inline void printTime(time_t time);

void printBlock(block *target);

pack *newPack(char *dn, uint64_t xl, char *xt, char *tr);

tran *newTran();

block *newBlock(uint64_t key, uint32_t nPack, pack **packs);

chain *newChain(void);

//! return 1 on success
bool insertBlock(block *bx, chain *ch);

uint32_t deletePack(pack *target);

uint32_t deleteBlock(block *target);

uint32_t deleteChain(chain *target);

#endif//_ALIB_H

