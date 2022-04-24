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
	if (!px || !fp || !buf) return false;

	len = 9 + MAGNET_XT_LEN;
	if (len != fread(buf, 1, len, fp)) return false;
	if (buf[0] != 'P') return false;
	memcpy(xt, buf + 1, MAGNET_XT_LEN);
	u64Unpack(buf + 1 + MAGNET_XT_LEN, &xl);

	if (1 != fread(buf, 1, 1, fp)) return false;
	len = buf[0];
	if (len > MAX_U8 || len != fread(buf, 1, len, fp)) return false;
	memcpy(dn, buf, len);
	dn[len] = 0;

	if (1 != fread(buf, 1, 1, fp)) return false;
	len = buf[0];
	if (len > MAX_U8 || len != fread(buf, 1, len, fp)) return false;
	memcpy(tr, buf, len);
	tr[len] = 0;

	// TODO: add kt
	return newPack(px, xt, xl, dn, tr, kt);
}

bool tranFromZip(tran *tx, FILE *fp, uint8_t *buf)
{
	uint64_t time, id, amount, src, dest;
	if (!tx || !fp || !buf) return false;

	if (41 != fread(buf, 1, 41, fp)) return false;
	if (buf[0] != 'T') return false;
	u64Unpack(buf + 1,  &time);
	u64Unpack(buf + 9,  &id);
	u64Unpack(buf + 17, &amount);
	u64Unpack(buf + 25, &src);
	u64Unpack(buf + 33, &dest);

	return newTran(tx, time, id, amount, src, dest);
}

bool blockFromZip(chain *ch, FILE *fp, uint8_t *buf)
{
	if (!ch || !fp || !buf) return false;
	bool ret;
	uint8_t crc[SHA512_LEN], key[SHA512_LEN];
	uint32_t i = 0, nRead, n_packs, n_trans;
	uint64_t n, time;
	pack *packs;
	tran *trans;

	nRead = 1 + 2 * SHA512_LEN;
	if (nRead != fread(buf, 1, nRead, fp)) return false;
	if (buf[0] != 'B') return false;
	memcpy(crc, buf + 1, SHA512_LEN);
	memcpy(key, buf + 1 + SHA512_LEN, SHA512_LEN);
	printf("c%02X%02X ", crc[0], crc[1]);
	printf("k%02X%02X ", key[0], key[1]);

	if (18 != fread(buf, 1, 18, fp)) return false;
	u64Unpack(buf, &n);
	u64Unpack(buf + 8, &time);
	n_packs = buf[16];
	n_trans = buf[17];
	packs = (pack *) calloc(n_packs, sizeof(pack));
	trans = (tran *) calloc(n_trans, sizeof(tran));

	for (i = 0; i < n_packs; i++) if (!packFromZip(&packs[i], fp, buf)) return false;
	for (i = 0; i < n_trans; i++) if (!tranFromZip(&trans[i], fp, buf)) return false;
	// TODO: run a sha3-512 check to see if crc matches

	ret = insertBlock(ch, n, time, n_packs, packs, n_trans, trans, crc, key);
	if (packs) free(packs);
	if (trans) free(trans);
	return ret;
}

bool chainFromZip(chain *ch, const char *dest)
{
	bool ret = false;
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
		if (!blockFromZip(ch, fp, buf))
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
