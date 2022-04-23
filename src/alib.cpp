#include <algorithm>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

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

uint32_t u64Unpack(uint8_t *buf, uint64_t *data)
{
	*data = 0;
	for (uint32_t i = 0; i < 8; i++)
	{
		*data |= (uint64_t) buf[i] << (i * 8);
	}
	return 8U;
}

bool sha512_copy(uint8_t *dest, uint8_t *md_val, uint32_t shaLen)
{
	if (!md_val || shaLen != SHA512_LEN)
	{
		if (md_val) free(md_val);
		printf("SHA3-512 for crc failed. Bytes expected=%d, actual=%u\n", SHA512_LEN, shaLen);
		return false;
	}
	memcpy(dest, md_val, SHA512_LEN);
	free(md_val);
	return true;
}

void printBlock(block *target)
{
	//printTime((time_t) target->time);
	printf("B%04lu %lu P%u T%u\n", target->n, target->time, target->n_packs, target->n_trans);
}

bool newPack(pack *px, uint8_t xt[MAGNET_XT_LEN], uint64_t xl, char *dn, char *tr, char *kt[MAGNET_KT_COUNT])
{
	uint32_t ndn = strlen(dn) + 1;
	uint32_t ntr = strlen(tr) + 1;
	uint32_t nkt, i, j;

	if (ndn > MAX_U8 || ntr > MAX_U8) return false;

	memcpy(px->xt, xt, MAGNET_XT_LEN);
	px->xl = xl;
	if (!(px->dn = (char *) calloc(ndn, sizeof(char)))) goto cleanup;
	if (!(px->tr = (char *) calloc(ntr, sizeof(char)))) goto cleanup;
	// TODO: check input for special characters, make it match packToZip
	// TODO: xt has to be 160 bit checksum
	strcpy(px->dn, dn);
	strcpy(px->tr, tr);

	for (i = 0; i < MAGNET_KT_COUNT; ++i) px->kt[i] = NULL;
	for (i = 0, j = 0; i < MAGNET_KT_COUNT; ++i)
	{
		if (!kt[i]) break;
		nkt = strlen(kt[i]);
		if (nkt > MAGNET_KT_LEN) continue;
		if (!(px->kt[j] = (char *) calloc(nkt + 1, sizeof(char)))) continue;
		strcpy(px->kt[j], kt[i]);
		++j;
	}

	return true;
cleanup:
	if (px->dn) free(px->dn);
	if (px->tr) free(px->tr);
	return false;
}

bool newTran(tran *tx, uint64_t time, uint64_t id, uint64_t amount, uint64_t src, uint64_t dest)
{
	// TODO: add error checks
	tx->time = time;
	tx->id = id;
	tx->amount = amount;
	tx->src = src;
	tx->dest = dest;
	return true;
}

bool newBlock(block *bx, uint64_t n, uint64_t time,
			  uint32_t n_packs, pack *packs,
			  uint32_t n_trans, tran *trans)
{
	if (!bx || n_packs > MAX_U8 || n_trans > MAX_U8) return false;
	uint8_t *md_val;
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

	md_val = finish_sha3_512(&shaLen, md_ctx);
	if (!sha512_copy(bx->crc, md_val, shaLen)) return false;

	// TODO: fix this broken key gen
	md_val = finish_sha3_512(bx->crc, SHA512_LEN, &shaLen);
	if (!sha512_copy(bx->key, md_val, shaLen)) return false;

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
	if (target->tr) free(target->tr);
	for (uint32_t i = 0; i < MAGNET_KT_COUNT; ++i) if (target->kt[i]) free(target->kt[i]);
}
 
void deleteTran(tran *target)
{
}

void deleteBlock(block *target)
{
	for (uint32_t i = 0; i < target->n_packs; ++i) deletePack(&(target->packs[i]));
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
