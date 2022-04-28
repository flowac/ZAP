/**
 * @file alib_wallet.h
 * @brief Functions that deal with wallet actions
 */
#ifndef _ALIB_WALLET_H
#define _ALIB_WALLET_H

#include "atype.h"

bool newWallet(uint8_t pub[ED448_LEN], uint8_t priv[ED448_LEN], const char *seed = NULL);
bool sendToAddress(uint8_t dest[ED448_LEN], uint8_t src[ED448_LEN], uint64_t deci, uint16_t frac);

#endif //_ALIB_WALLET_H
