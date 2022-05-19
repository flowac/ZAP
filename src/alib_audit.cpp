#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include "alib.h"
#include "log.h"
#include "ssl_fn.h"
#include "time_fn.h"

bool checkBlock(block *bx, bool modify, uint8_t crc[SHA3_LEN])
{
	if (!bx || (bx->n_packs && !bx->packs) || (bx->n_trans && !bx->trans)) return false;
	uint8_t *md_val;
	uint32_t shaLen;
	EVP_MD_CTX *md_ctx;

	uint8_t bitPack[18];
	uint32_t i, j, len;
	u64Packer(bitPack, bx->n);
	u64Packer(bitPack + 8, bx->time);
	bitPack[16] = bx->n_packs;
	bitPack[17] = bx->n_trans;

	if (!(md_ctx = update_sha3(bitPack, 18, NULL))) return false;
	if (!update_sha3(bx->key, SHA3_LEN, md_ctx)) return false;

	if (bx->n_packs)
	{
		if (!bx->packs) return false;
		for (i = 0; i < bx->n_packs; ++i)
		{
			if (!update_sha3(bx->packs[i].xt, MAGNET_XT_LEN, md_ctx)) return false;
			if (!update_sha3(&(bx->packs[i].xl), 8, md_ctx)) return false;

			if (0 == (len = strlen(bx->packs[i].dn))) return false;
			if (!update_sha3(bx->packs[i].dn, len, md_ctx)) return false;
			if (0 == (len = u8len(bx->packs[i].tr))) return false;
			if (!update_sha3(bx->packs[i].tr, len, md_ctx)) return false;

			for (j = 0; j < MAGNET_KT_COUNT; ++j)
			{
				if (!bx->packs[i].kt[j] || 0 == (len = strlen(bx->packs[i].kt[j]))) break;
				if (!update_sha3(bx->packs[i].kt[j], len, md_ctx)) return false;
			}
		}
	}

	if (bx->n_trans)
	{
		if (!bx->trans) return false;
		for (i = 0; i < bx->n_trans; ++i)
		{
			u64Packer(bitPack, bx->trans[i].id);
			u64Packer(bitPack + 8, bx->trans[i].deci);
			u16Packer(bitPack + 16, bx->trans[i].frac);
			if (!update_sha3(bitPack, 18, md_ctx)) return false;
			if (!update_sha3(bx->trans[i].src, ED448_LEN, md_ctx)) return false;
			if (!update_sha3(bx->trans[i].dest, ED448_LEN, md_ctx)) return false;
			if (!update_sha3(bx->trans[i].sig, ED448_SIG_LEN, md_ctx)) return false;
		}
	}

	if (crc && !update_sha3(crc, SHA3_LEN, md_ctx)) return false;
	md_val = finish_sha3(&shaLen, md_ctx);
	if (modify)	return sha3_copy_free(bx->crc, md_val, shaLen);
	return sha3_cmp_free(bx->crc, md_val);
}

bool auditChain(chain *ch)
{
	// TODO: add balance checks
	for (uint64_t i = 0; i < ch->blk.size(); ++i)
	{
		if (!checkBlock(&(ch->blk[i]), false, i ? ch->blk[i-1].crc : NULL)) return false;
	}

	return true;
}

uint64_t compareChain(chain *left, chain *right)
{
	// TODO: add balance compares
	uint64_t lLen = left->blk.size();
	uint64_t rLen = right->blk.size();
	uint64_t lrMin = std::min(lLen, rLen);
	uint64_t ret = (lLen == rLen) ? MAX_U64 : lrMin;

	for (uint64_t i = 0; i < lrMin; ++i)
	{
		if (!sha3_cmp(left->blk[i].crc, right->blk[i].crc)) return i;
	}

	return ret;
}
