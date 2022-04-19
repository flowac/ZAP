#include <algorithm>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "alib.h"
#include "log.h"
#include "ssl_fn.h"

// TODO: move the two below to time_fn.cpp
time_t sNow()
{
	time_t raw;
	time(&raw);
	return raw;
}

void printTime(time_t time)
{
	char buf[MAX_U8];
	strftime(buf, MAX_U8, "%g %B %d  %H:%M:%S", localtime(&time));
}

uint32_t u64Packer(uint8_t *buf, uint64_t data)
{
	for (uint32_t i = 0; i < 8; i++)
	{
		buf[i] = (data >> (i * 8)) & MAX_U8;
	}
	return 8U;
}

void printBlock(block *target)
{
	//printTime((time_t) target->time);
	printf("B%04lu %lu P%u T%u\n", target->n, target->time, target->n_packs, target->n_trans);
}

bool newPack(pack *px, char *dn, uint64_t xl, char *xt, char *tr)
{
	uint32_t ndn = strlen(dn) + 1;
	uint32_t nxt = strlen(xt) + 1;
	uint32_t ntr = strlen(tr) + 1;

	if (ndn > MAX_U8 || nxt > MAX_U8 || ntr > MAX_U8)
		return false;

	px->xl = xl;
	strncpy(px->info, dn, 5);
	px->info[5] = 0;

	px->dn = (char *) malloc(sizeof(char) * ndn);
	px->xt = (char *) malloc(sizeof(char) * nxt);
	px->tr = (char *) malloc(sizeof(char) * ntr);

	if (!px->dn || !px->xt || !px->tr)
		return false;

	strcpy(px->dn, dn);
	strcpy(px->xt, xt);
	strcpy(px->tr, tr);

	return true;
}

void newTran(tran *tx)
{
}

bool newBlock(block *bx, uint64_t n, uint64_t time,
			  uint32_t n_packs, pack *packs,
			  uint32_t n_trans, tran *trans)
{
	if (!bx || n_packs > MAX_U8 || n_trans > MAX_U8) return false;
	uint32_t shaLen;
	EVP_MD_CTX *md_ctx;

	bx->n = n;
	bx->time = time ? time : (uint64_t) sNow();
	bx->n_packs = n_packs;
	bx->n_trans = n_trans;
	bx->packs = packs;
	bx->trans = trans;

	uint8_t bitPack[18];
	u64Packer(bitPack, bx->n);
	u64Packer(bitPack + 8, bx->time);
	bitPack[16] = bx->n_packs;
	bitPack[17] = bx->n_trans;

	if (!(md_ctx = update_sha3_512(bitPack, 18, NULL))) return false;

	// TODO: Do these checksums properly instead of taking a whole chunk of memory
	if (bx->packs) if (!update_sha3_512(bx->packs, sizeof(pack) * bx->n_packs, md_ctx)) return false;
	if (bx->trans) if (!update_sha3_512(bx->trans, sizeof(tran) * bx->n_trans, md_ctx)) return false;

	bx->crc = finish_sha3_512(&shaLen, md_ctx);
	if (shaLen != 64)
	{
		if (bx->crc) free(bx->crc);
		printf("SHA3-512 for crc failed. Bytes expected=64, actual=%u\n", shaLen);
		return false;
	}

	// TODO: fix this broken key gen
	bx->key = finish_sha3_512(bx->crc, 64, &shaLen);
	if (shaLen != 64)
	{
		if (bx->key) free(bx->key);
		free(bx->crc);
		printf("SHA3-512 for key failed. Bytes expected=64, actual=%u\n", shaLen);
		return false;
	}

	if (LOG) printTime(bx->time);
	return true;
}

chain *newChain(void)
{
	chain *ch = (chain *) malloc(sizeof(chain));
	if (!ch) return ch;

	ch->n_bal = 0;
	ch->n_blk = 0;
	ch->bal = NULL;
	return ch;
}

//! return 1 on success
//TODO: compact blocks once full
bool insertBlock(block *bx, chain *ch)
{
	if (!bx || !ch) return false;

	if (ch->n_blk < B_MAX) {
		ch->blk[ch->n_blk] = *bx;
		++ch->n_blk;
	}
	else
	{
		log_msg_default;
		return false;
	}
	return true;
}

void deletePack(pack *target)
{
	if (target->dn) free(target->dn);
	if (target->xt) free(target->xt);
	if (target->tr) free(target->tr);
}

void deleteBlock(block *target)
{
	for (uint32_t i = 0; i < target->n_packs; i++) deletePack(&(target->packs[i]));
	if (target->key)   free(target->key);
	if (target->crc)   free(target->crc);
	if (target->packs) free(target->packs);
	if (target->trans) free(target->trans);
}

void deleteChain(chain *target)
{
	for (uint32_t i = 0; i < target->n_blk; i++) deleteBlock(&(target->blk[i]));
	if (target->bal) free(target->bal);
}
