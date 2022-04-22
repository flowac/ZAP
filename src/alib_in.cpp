#include <stdlib.h>
#include <string.h>

#include "alib.h"
#include "alib_io.h"

// TODO: add kt
bool packFromZip(pack *pk, FILE *fp, uint8_t *buf)
{
	uint32_t len;
	if (!pk || !fp || !buf) return false;

	len = 9 + MAGNET_XT_LEN;
	if (len != fread(buf, 1, len, fp)) return false;
	if (buf[0] != 'P') return false;
	memcpy(pk->xt, buf + 1, MAGNET_XT_LEN);
	u64Unpack(buf + 1 + MAGNET_XT_LEN, &pk->xl);

	len = fread(buf, 1, 1, fp);
	if (len != fread(buf, 1, len, fp)) return false;
	pk->dn = (char *) calloc(len + 1, 1);
	memcpy(pk->dn, buf, len);

	len = fread(buf, 1, 1, fp);
	if (len != fread(buf, 1, len, fp)) return false;
	pk->tr = (char *) calloc(len + 1, 1);
	memcpy(pk->tr, buf, len);

	// TODO: add kt

	return true;
}

bool tranFromZip(tran *tx, FILE *fp, uint8_t *buf)
{
	if (!tx || !fp || !buf) return false;

	if (41 != fread(buf, 1, 41, fp)) return false;
	if (buf[0] != 'T') return false;
	u64Unpack(buf + 1,  &tx->time);
	u64Unpack(buf + 9,  &tx->id);
	u64Unpack(buf + 17, &tx->amount);
	u64Unpack(buf + 25, &tx->src);
	u64Unpack(buf + 33, &tx->dest);

	return true;
}

bool blockFromZip(block *bx, FILE *fp, uint8_t *buf)
{
	uint32_t i = 0, nRead;
	if (!bx || !fp || !buf) return false;

	nRead = 1 + 2 * SHA512_LEN;
	if (nRead != fread(buf, 1, nRead, fp)) return false;
	if (buf[0] != 'B') return false;
	memcpy(bx->key, buf + 1, SHA512_LEN);
	memcpy(bx->crc, buf + 1 + SHA512_LEN, SHA512_LEN);

	if (18 != fread(buf, 1, 18, fp)) return false;
	u64Unpack(buf, &bx->n);
	u64Unpack(buf + 8, &bx->time);
	bx->n_packs = buf[16];
	bx->n_trans = buf[17];

	for (i = 0; i < bx->n_packs; i++) if (!packFromZip(&(bx->packs[i]), fp, buf)) return false;
	for (i = 0; i < bx->n_trans; i++) if (!tranFromZip(&(bx->trans[i]), fp, buf)) return false;
	// TODO: run a sha3-512 check to see if crc matches

	return true;
}

bool chainFromZip(chain *ch, const char *dest)
{
	FILE *fp;
	uint8_t buf[BUF1K];
	uint64_t chainLen;
	if (!ch || !dest || !(fp = fopen(dest, "wb"))) return false;

	memset(buf, 0, BUF1K);
	if (9 != fread(buf, 1, 9, fp)) return false;
	if (buf[0] != 'C') return false;
	u64Unpack(buf + 1, &chainLen);

	for (uint64_t i = 0; i < chainLen; ++i)
	{
		if (blockFromZip(&(ch->blk[i]), fp, buf)) continue;
		fclose(fp);
		deleteChain(ch);
		return false;
	}

	fclose(fp);
	return true;
}
