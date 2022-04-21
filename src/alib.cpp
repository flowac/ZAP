#include <algorithm>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "alib.h"
#include "log.h"
#include "ssl_fn.h"
#include "time_fn.h"

static std::queue<pack *> pack_queue;
static std::queue<tran *> tran_queue;

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

bool newPack(pack *px, uint64_t xl, char *dn, char *xt, char *tr, char *kt[KEYWORD_TOPIC_COUNT])
{
	uint32_t ndn = strlen(dn) + 1;
	uint32_t nxt = strlen(xt) + 1;
	uint32_t ntr = strlen(tr) + 1;
	uint32_t nkt;

	if (ndn > MAX_U8 || nxt > MAX_U8 || ntr > MAX_U8)
		return false;

	px->xl = xl;
	if (!(px->dn = (char *) calloc(ndn, sizeof(char)))) goto cleanup;
	if (!(px->xt = (char *) calloc(nxt, sizeof(char)))) goto cleanup;
	if (!(px->tr = (char *) calloc(ntr, sizeof(char)))) goto cleanup;
	strcpy(px->dn, dn);
	strcpy(px->xt, xt);
	strcpy(px->tr, tr);

	for (uint32_t i = 0; i < KEYWORD_TOPIC_COUNT; ++i) px->kt[i] = NULL;
	for (uint32_t i = 0, j = 0; i < KEYWORD_TOPIC_COUNT; ++i)
	{
		if (!kt[i]) break;
		nkt = strlen(kt[i]) + 1;
		if (nkt > MAX_U8) continue;
		if (!(px->kt[j] = (char *) calloc(nkt, sizeof(char)))) continue;
		strcpy(px->kt[j], kt[i]);
		++j;
	}

	return true;
cleanup:
	if (px->dn) free(px->dn);
	if (px->xt) free(px->xt);
	if (px->tr) free(px->tr);
	return false;
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

//TODO: compact blocks once full
//TODO: validate blocks or merge with newBlock
bool insertBlock(block *bx, chain *ch)
{
	if (!bx || !ch) return false;

	ch->blk.push_back(*bx);
	if (ch->blk.size() > B_MAX)
	{
		printf("Max number of blocks %d exceeded. Currently %lu.\n", B_MAX, ch->blk.size());
	}

	return true;
}

void deletePack(pack *target)
{
	if (target->dn) free(target->dn);
	if (target->xt) free(target->xt);
	if (target->tr) free(target->tr);
	for (uint32_t i = 0; i < KEYWORD_TOPIC_COUNT; ++i) if (target->kt[i]) free(target->kt[i]);
}
 
void deleteTran(tran *target)
{
}

void deleteBlock(block *target)
{
	for (uint32_t i = 0; i < target->n_packs; ++i) deletePack(&(target->packs[i]));
	if (target->key)   free(target->key);
	if (target->crc)   free(target->crc);
	if (target->packs) free(target->packs);
	if (target->trans) free(target->trans);
}

void deleteChain(chain *target)
{
	target->bal.clear();
	for (uint64_t i = 0; i < target->blk.size(); ++i) deleteBlock(&(target->blk[i]));
	target->blk.clear();
}

bool enqueuePack(pack *target)
{
	if (!target) return false;
	pack_queue.push(target);
	return true;
}

bool enqueueTran(tran *target)
{
	if (!target) return false;
	tran_queue.push(target);
	return true;
}

pack *dequeuePack(void)
{
	if (pack_queue.empty()) return NULL;
	pack *target = pack_queue.front();
	pack_queue.pop();
	return target;
}

tran *dequeueTran(void)
{
	if (tran_queue.empty()) return NULL;
	tran *target = tran_queue.front();
	tran_queue.pop();
	return target;
}
