#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include "main_lib.h"
#include "log.h"
#include "ssl_fn.h"
#include "time_fn.h"

bool checkPack(pack *px, bool modify)
{
	uint8_t *md_val = NULL;
	uint32_t shaLen;
	EVP_MD_CTX *md_ctx = NULL;

	if (!px || !px->dn || !px->tr) goto cleanup;
	if (!(md_ctx = update_shake(px->xt, MAGNET_XT_LEN, NULL))) goto cleanup;
	if (!(update_shake(&(px->xl), 8, md_ctx))) goto cleanup;
	if (!(update_shake(px->dn, strlen(px->dn), md_ctx))) goto cleanup;
	if (!(update_shake(px->tr,  u8len(px->tr), md_ctx))) goto cleanup;
	if (!(update_shake(px->kt, 1, md_ctx))) goto cleanup;

	if (!(md_val = finish_shake(&shaLen, &md_ctx))) goto cleanup;
	if (modify) return shake_copy_free(px->crc, md_val, shaLen);
	return shake_cmp_free(px->crc, md_val);
cleanup:
	if (md_ctx) EVP_MD_CTX_free(md_ctx);
	return false;
}

bool auditTorDB(torDB *td)
{
	for (uint64_t i = 0; i < td->pak.size(); ++i)
		if (!checkPack(&(td->pak[i]), false)) return false;
	return true;
}

bool checkBlock(block *bx, bool modify, uint8_t crc[SHA3_LEN])
{
	uint8_t bitPack[18], *md_val = NULL;
	uint32_t i, shaLen;
	EVP_MD_CTX *md_ctx = NULL;

	if (!bx || (bx->n_trans && !bx->trans)) goto cleanup;
	u64Packer(bitPack, bx->n);
	u64Packer(bitPack + 8, bx->time);
	bitPack[9] = bx->n_trans;

	if (!(md_ctx = update_sha3(bitPack, 10, NULL))) goto cleanup;
	if (!update_sha3(bx->key, SHA3_LEN, md_ctx)) goto cleanup;

	if (bx->n_trans)
	{
		if (!bx->trans) goto cleanup;
		for (i = 0; i < bx->n_trans; ++i)
		{
			u64Packer(bitPack, bx->trans[i].id);
			u64Packer(bitPack + 8, bx->trans[i].deci);
			u16Packer(bitPack + 16, bx->trans[i].frac);
			if (!update_sha3(bitPack, 18, md_ctx)) goto cleanup;
			if (!update_sha3(bx->trans[i].src,  ED448_LEN, md_ctx)) goto cleanup;
			if (!update_sha3(bx->trans[i].dest, ED448_LEN, md_ctx)) goto cleanup;
			if (!update_sha3(bx->trans[i].sig,  ED448_SIG_LEN, md_ctx)) goto cleanup;
		}
	}

	// Previous block hash for the merkle tree
	if (crc && !update_sha3(crc, SHA3_LEN, md_ctx)) goto cleanup;
	if (!(md_val = finish_sha3(&shaLen, &md_ctx))) goto cleanup;
	if (modify) return sha3_copy_free(bx->crc, md_val, shaLen);
	return sha3_cmp_free(bx->crc, md_val);
cleanup:
	if (md_ctx) EVP_MD_CTX_free(md_ctx);
	return false;
}

bool auditChain(chain *ch)
{
	// TODO: add balance checks
	for (uint64_t i = 0; i < ch->blk.size(); ++i)
		if (!checkBlock(&(ch->blk[i]), false, i ? ch->blk[i-1].crc : NULL)) return false;
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
