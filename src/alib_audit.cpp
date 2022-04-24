#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include "alib.h"
#include "log.h"
#include "ssl_fn.h"
#include "time_fn.h"

bool checkBlock(block *bx, bool modify)
{
	if (!bx || (bx->n_packs && !bx->packs) || (bx->n_trans && !bx->trans)) return false;
	uint8_t *md_val;
	uint32_t shaLen;
	EVP_MD_CTX *md_ctx;

	uint8_t bitPack[18];
	u64Packer(bitPack, bx->n);
	u64Packer(bitPack + 8, bx->time);
	bitPack[16] = bx->n_packs;
	bitPack[17] = bx->n_trans;

	if (!(md_ctx = update_sha3_512(bitPack, 18, NULL))) return false;

	// TODO: Do these checksums properly instead of taking a whole chunk of memory
//	if (bx->packs) if (!update_sha3_512(bx->packs, sizeof(pack) * bx->n_packs, md_ctx)) return false;
//	if (bx->trans) if (!update_sha3_512(bx->trans, sizeof(tran) * bx->n_trans, md_ctx)) return false;
	if (!update_sha3_512(bx->key, SHA512_LEN, md_ctx)) return false;

	md_val = finish_sha3_512(&shaLen, md_ctx);
	if (modify)	return sha512_copy_free(bx->crc, md_val, shaLen);
	return sha512_cmp_free(bx->crc, md_val);
}

bool auditChain(chain *ch)
{
	// TODO: add balance checks
	for (uint64_t i = 0; i < ch->blk.size(); ++i)
	{
		if (!checkBlock(&(ch->blk[i]))) return false;
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
		if (!sha512_cmp(left->blk[i].crc, right->blk[i].crc)) return i;
	}

	return ret;
}
