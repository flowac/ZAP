#include <algorithm>
#include <queue>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "main_lib.h"
#include "log.h"
#include "ssl_fn.h"
#include "time_fn.h"

static std::queue<tran> tran_queue;

uint32_t u16Packer(uint8_t *buf, uint16_t data)
{
	buf[0] = data & MAX_U8;
	buf[1] = (data >> 8) & MAX_U8;
	return 2U;
}

uint32_t u16Unpack(uint8_t *buf, uint16_t *data)
{
	*data = (uint16_t) buf[0];
	*data |= (uint16_t) buf[1] << 8;
	return 2U;
}

uint32_t u64Packer(uint8_t *buf, uint64_t data)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		buf[i] = (data >> (i * 8)) & MAX_U8;
	}
	return 8U;
}

uint32_t u64Unpack(uint8_t *buf, uint64_t *data)
{
	*data = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		*data |= (uint64_t) buf[i] << (i * 8);
	}
	return 8U;
}

uint32_t u8len(uint8_t *ptr)
{
	uint32_t len = 0;
	for (; *ptr; ++len, ++ptr);
	return len;
}

uint32_t u8cmp(uint8_t *ptr, char *str)
{
	uint32_t len = 0;
	for (; *str; ++len, ++ptr, ++str) if (*ptr != *str) return 0;
	return len;
}

bool enqueueTran(tran *target)
{
	if (!target) return false;
	tran_queue.push(*target);
	return true;
}

bool dequeueTran(tran *target)
{
	if (tran_queue.empty()) return false;
	*target = tran_queue.front();
	tran_queue.pop();
	return true;
}

bool newTran(tran *tx, uint64_t id, uint64_t deci, uint16_t frac,
			 uint8_t src[ED448_LEN], uint8_t dest[ED448_LEN], uint8_t sig[ED448_SIG_LEN])
{
	tran ltx, *ptx;
	if (!id || !(deci || frac) || !src || !dest || !sig) return false;

	// TODO: verify sig
	if (tx) ptx = tx;
	else ptx = &ltx;
	ptx->id = id;
	ptx->deci = deci;
	ptx->frac = frac;
	memcpy(ptx->src,  src,  ED448_LEN);
	memcpy(ptx->dest, dest, ED448_LEN);
	memcpy(ptx->sig,  sig,  ED448_SIG_LEN);

	if (tx) return true;
	return enqueueTran(ptx);
}

bool trimBlock(chain *ch)
{
	if (ch->blk.size() < B_MAX) return true;

	printf("Max number of blocks %d reached. Currently %lu.\n", B_MAX, ch->blk.size());
	return true;
}

bool newBlock(chain *ch)
{
	if (!ch || (false && tran_queue.empty())) return false;
	uint8_t *md_val;
	uint32_t i, shaLen;
	block bx;

	bx.n = ch->blk.empty() ? 1 : ch->blk.back().n + 1;
	bx.time = nsNow();
	bx.n_trans = std::min(tran_queue.size(), MAX_U8);
	bx.trans = NULL;

	if (bx.n_trans)
	{
		if (!(bx.trans = (tran *) calloc(bx.n_trans, sizeof(tran)))) goto cleanup;
		for (i = 0; i < bx.n_trans; ++i)
		{
			if (!dequeueTran(&(bx.trans[i])))
			{
				bx.n_trans = i;
				bx.trans = (tran *) realloc(bx.trans, sizeof(tran) * bx.n_trans);
				break;
			}
		}
	}
	md_val = finish_sha3(&(bx.n), 8, &shaLen);
	if (!sha3_copy_free(bx.key, md_val, shaLen)) goto cleanup;
	if (!checkBlock(&bx, true, ch->blk.empty() ? NULL : ch->blk.back().crc)) goto cleanup;
	ch->blk.push_back(bx);

	return trimBlock(ch);
cleanup:
	if (bx.trans) free(bx.trans);
	return false;
}

bool insertBlock(chain *ch,
				 uint64_t n, uint64_t time,
				 uint32_t n_trans, tran *trans,
				 uint8_t crc[SHA3_LEN],
				 uint8_t key[SHA3_LEN])
{
	if (!ch || n_trans > MAX_U8) return false;
	if (ch->blk.size() != 0 && n != (ch->blk.back().n + 1)) return false;
	uint8_t *md_val;
	uint32_t shaLen;
	block bx;

	bx.n = n;
	bx.time = time ? time : nsNow();
	bx.n_trans = n_trans;
	bx.trans = trans;

	// TODO: fix this broken key gen
	md_val = finish_sha3(&(bx.n), 8, &shaLen);
	if (key) if (!sha3_cmp(key, md_val)) return false;
	if (!sha3_copy_free(bx.key, md_val, shaLen)) return false;
	if (crc && !sha3_copy(bx.crc, crc, SHA3_LEN)) return false;

	if (!checkBlock(&bx, crc == NULL, ch->blk.empty() ? NULL : ch->blk.back().crc)) return false;
	ch->blk.push_back(bx);
	return trimBlock(ch);
}

inline void deleteTran(tran *target)
{
}

inline void deleteBlock(block *target)
{
	if (!target) return;
	for (uint32_t i = 0; i < target->n_trans; ++i) deleteTran(&(target->trans[i]));
	if (target->trans) free(target->trans);
}

void deleteChain(chain *target)
{
	uint64_t i;
	target->bal.clear();
	for (i = 0; i < target->blk.size(); ++i) deleteBlock(&(target->blk[i]));
	target->blk.clear();
}
