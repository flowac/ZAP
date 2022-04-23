#include <stdlib.h>
#include <string.h>

#include "alib.h"
#include "alib_io.h"

// TODO: add kt
bool packFromZip(pack *px, FILE *fp, uint8_t *buf)
{
	uint8_t xt[MAGNET_XT_LEN];
	uint32_t len;
	uint64_t xl;
	char dn[MAX_U8 + 1];
	char tr[MAX_U8 + 1];
	char *kt[MAGNET_KT_COUNT] = {NULL, NULL, NULL, NULL, NULL};
	printf("p ");
	if (!px || !fp || !buf) return false;

	printf("p0 ");
	len = 9 + MAGNET_XT_LEN;
	if (len != fread(buf, 1, len, fp)) return false;
	if (buf[0] != 'P') return false;
	memcpy(xt, buf + 1, MAGNET_XT_LEN);
	u64Unpack(buf + 1 + MAGNET_XT_LEN, &xl);

	printf("p1 ");
	len = fread(buf, 1, 1, fp);
	if (len > MAX_U8 || len != fread(buf, 1, len, fp)) return false;
	memcpy(dn, buf, len);
	dn[len] = 0;

	printf("p2 ");
	len = fread(buf, 1, 1, fp);
	if (len > MAX_U8 || len != fread(buf, 1, len, fp)) return false;
	memcpy(tr, buf, len);
	tr[len] = 0;

	// TODO: add kt
	return newPack(px, xt, xl, dn, tr, kt);
}

bool tranFromZip(tran *tx, FILE *fp, uint8_t *buf)
{
	uint64_t time, id, amount, src, dest;
	printf("t ");
	if (!tx || !fp || !buf) return false;

	printf("t0 ");
	if (41 != fread(buf, 1, 41, fp)) return false;
	if (buf[0] != 'T') return false;
	u64Unpack(buf + 1,  &time);
	u64Unpack(buf + 9,  &id);
	u64Unpack(buf + 17, &amount);
	u64Unpack(buf + 25, &src);
	u64Unpack(buf + 33, &dest);

	return newTran(tx, time, id, amount, src, dest);
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
	bx->packs = (pack *) calloc(bx->n_packs, sizeof(pack));
	bx->trans = (tran *) calloc(bx->n_trans, sizeof(tran));

	for (i = 0; i < bx->n_packs; i++) if (!packFromZip(&(bx->packs[i]), fp, buf)) return false;
	for (i = 0; i < bx->n_trans; i++) if (!tranFromZip(&(bx->trans[i]), fp, buf)) return false;
	// TODO: run a sha3-512 check to see if crc matches

	return true;
}

bool chainFromZip(chain *ch, const char *dest)
{
	bool ret = false;
	block bx;
	FILE *fp;
	uint8_t buf[BUF1K];
	uint64_t chainLen;
	if (!ch || !dest || !(fp = fopen(dest, "rb"))) goto cleanup;

	memset(buf, 0, BUF1K);
	if (9 != fread(buf, 1, 9, fp)) goto cleanup;
	if (buf[0] != 'C') goto cleanup;
	u64Unpack(buf + 1, &chainLen);

	for (uint64_t i = 0; i < chainLen; ++i)
	{
		printf("%lu ", i);
		if (!blockFromZip(&bx, fp, buf) || !insertBlock(&bx, ch))
		{
			deleteChain(ch);
			goto cleanup;
		}
	}

	ret = true;
cleanup:
	if (fp) fclose(fp);
	return ret;
}
