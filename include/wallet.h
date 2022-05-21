/**
 * @file alib_wallet.h
 * @brief Functions that deal with wallet actions
 */
#ifndef _ALIB_WALLET_H
#define _ALIB_WALLET_H

#include "types.h"

bool newWallet(uint8_t pub[ED448_LEN], uint8_t priv[ED448_LEN], const char *seed = NULL);
bool sendToAddress(uint8_t dest[ED448_LEN], uint8_t src[ED448_LEN], uint8_t sig[ED448_SIG_LEN], uint64_t deci, uint16_t frac);
bool verifyMessage(uint8_t pub[ED448_LEN], uint8_t sig[ED448_SIG_LEN], uint8_t *msg, uint64_t msgLen);

#endif //_ALIB_WALLET_H
