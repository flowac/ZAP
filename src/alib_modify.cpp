#include <algorithm>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "alib.h"
#include "log.h"
#include "ssl_fn.h"
#include "time_fn.h"

// TODO: when calling newBlock, use these queues instead of taking parameters
static std::queue<pack> pack_queue;
static std::queue<tran> tran_queue;

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

void printBlock(block *target)
{
	//printTime((time_t) target->time);
	printf("B%04lu %lu P%u T%u\n", target->n, target->time, target->n_packs, target->n_trans);
}

bool newPack(pack *px, uint8_t xt[MAGNET_XT_LEN], uint64_t xl, char *dn, char *tr, char *kt[MAGNET_KT_COUNT])
{
	uint32_t ndn = 0;
	uint32_t ntr = 0;
	uint32_t nkt = 0, i = 0;
	if (!px || !dn || !tr) goto cleanup;
	px->dn = px->tr = NULL;
	if (!(ndn = strlen(dn)) || !(ntr = strlen(tr)) || ndn > MAGNET_DN_LEN || ntr > MAGNET_TR_LEN) goto cleanup;
	for (int i = 0; i < MAGNET_KT_COUNT; ++i) px->kt[i] = NULL;

	memcpy(px->xt, xt, MAGNET_XT_LEN);
	px->xl = xl;
	if (!(px->dn = (char *) calloc(ndn + 1, 1))) goto cleanup;
	if (!(px->tr = (char *) calloc(ntr + 1, 1))) goto cleanup;
	memcpy(px->dn, dn, ndn);
	memcpy(px->tr, tr, ntr);

	for (i = 0; i < MAGNET_KT_COUNT; ++i) px->kt[i] = NULL;
	for (i = 0; i < MAGNET_KT_COUNT; ++i)
	{
		if (!kt[i] || 0 == (nkt = strlen(kt[i]))) break;
		if (nkt > MAGNET_KT_LEN) goto cleanup;
		px->kt[i] = kt[i];
	}

//	printf("pack %s<\n", dn);
	return true;
cleanup:
	printf("\nfailed new pack kt[i] %u[%u] %s<\n", nkt, i, dn);
	if (kt) for (i = 0; i < MAGNET_KT_COUNT; i++) if (kt[i]) free(kt[i]);
	if (px)
	{
		if (px->dn) free(px->dn);
		if (px->tr) free(px->tr);
	}
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

bool newBlock(chain *ch)
{
	if (!ch || (pack_queue.empty() && tran_queue.empty())) return false;
	uint8_t *md_val;
	uint32_t i, shaLen;
	block bx;

	bx.n = ch->blk.size() + 1;
	bx.time = nsNow();
	bx.n_packs = std::min(pack_queue.size(), MAX_U8);
	bx.n_trans = std::min(tran_queue.size(), MAX_U8);
	bx.packs = NULL;
	bx.trans = NULL;

	if (bx.n_packs)
	{
		if (!(bx.packs = (pack *) calloc(bx.n_packs, sizeof(pack)))) goto cleanup;
		for (i = 0; i < bx.n_packs; ++i)
		{
			if (!dequeuePack(&(bx.packs[i])))
			{
				bx.n_packs = i + 1; // this is unlikely, therefore no realloc
				break;
			}
		}
	}

	if (bx.n_trans)
	{
		if (!(bx.trans = (tran *) calloc(bx.n_trans, sizeof(tran)))) goto cleanup;
		for (i = 0; i < bx.n_trans; ++i)
		{
			if (!dequeueTran(&(bx.trans[i])))
			{
				bx.n_trans = i + 1; // this is unlikely, therefore no realloc;
				break;
			}
		}
	}

	md_val = finish_sha3_512(&(bx.n), 8, &shaLen);
	if (!sha512_copy_free(bx.key, md_val, shaLen)) goto cleanup;
	if (!checkBlock(&bx, true)) goto cleanup;
	ch->blk.push_back(bx);

	return trimBlock(ch);
cleanup:
	if (bx.packs) free(bx.packs);
	if (bx.trans) free(bx.trans);
	return false;
}

bool insertBlock(chain *ch,
				 uint64_t n, uint64_t time,
				 uint32_t n_packs, pack *packs,
				 uint32_t n_trans, tran *trans,
				 uint8_t crc[SHA512_LEN],
				 uint8_t key[SHA512_LEN])
{
	if (!ch || n_packs > MAX_U8 || n_trans > MAX_U8) return false;
	if (ch->blk.size() != 0 && n != (ch->blk.back().n + 1)) return false;
	uint8_t *md_val;
	uint32_t shaLen;
	block bx;

	bx.n = n;
	bx.time = time ? time : nsNow();
	bx.n_packs = n_packs;
	bx.n_trans = n_trans;
	bx.packs = packs;
	bx.trans = trans;

	// TODO: fix this broken key gen
	md_val = finish_sha3_512(&(bx.n), 8, &shaLen);
	if (key) if (!sha512_cmp(key, md_val)) return false;
	if (!sha512_copy_free(bx.key, md_val, shaLen)) return false;
	if (crc && !sha512_copy(bx.crc, crc, SHA512_LEN)) return false;

	if (!checkBlock(&bx, crc == NULL)) return false;
	ch->blk.push_back(bx);
	return trimBlock(ch);
}

bool trimBlock(chain *ch)
{
	if (ch->blk.size() < B_MAX) return true;

	printf("Max number of blocks %d reached. Currently %lu.\n", B_MAX, ch->blk.size());
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
	for (uint32_t i = 0; i < target->n_trans; ++i) deleteTran(&(target->trans[i]));
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
	pack_queue.push(*target);
	return true;
}

bool enqueueTran(tran *target)
{
	if (!target) return false;
	tran_queue.push(*target);
	return true;
}

bool dequeuePack(pack *target)
{
	if (pack_queue.empty()) return false;
	*target = pack_queue.front();
	pack_queue.pop();
	return true;
}

bool dequeueTran(tran *target)
{
	if (tran_queue.empty()) return false;
	*target = tran_queue.front();
	tran_queue.pop();
	return true;
}
