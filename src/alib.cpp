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

void printBlock(block *target)
{
	uint32_t n_packs = target->n_packs;

	printTime((time_t) target->time);
	printf("[%lu] > 0x%lX [%u]\n", target->time, target->key, n_packs);

	if (!LOG)
		return;
	for (uint32_t i = 0; i < n_packs; i++) {
//        printf("%lX\t", target->packs[i]);
	}
	printf("\n");
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

bool newBlock(block *bx, uint32_t time, uint64_t n, uint64_t key, uint64_t *n_packs,
			  pack **packs)
{
	uint8_t *shaSum;
	uint32_t shaLen;
	bx->time = time ? time : (uint32_t) sNow();
	bx->n = n;
	bx->key = key;

	// TODO: fix this half-assed attempt, hash one string at a time
	// TODO: add n_trans
	shaSum = check_sha3_512(packs, std::max(*n_packs, MAX_U8), &shaLen);
	memcpy(bx->crc, shaSum, shaLen);
	free(shaSum);
	if (shaLen != 64)
	{
		printf("    crc check failed on block %lu\n", n);
		return false;
	}

	bx->packs = *packs;
	if (*n_packs > MAX_U8) {
		bx->n_packs = MAX_U8;
		*n_packs -= MAX_U8;
		*packs = &((*packs)[MAX_U8]);
	} else {
		bx->n_packs = *n_packs;
		*n_packs = 0;
		*packs = 0;
	}
	bx->n_trans = 0;
	bx->trans = 0;
	if (LOG)
		printTime(bx->time);

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
	if (target->packs != 0) free(target->packs);
	if (target->trans != 0) free(target->trans);
}

void deleteChain(chain *target)
{
	for (uint32_t i = 0; i < target->n_blk; i++) deleteBlock(&(target->blk[i]));
	if (target->bal) free(target->bal);
}
