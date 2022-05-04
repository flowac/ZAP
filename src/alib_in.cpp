#include <stdlib.h>
#include <string.h>

#include "alib.h"
#include "alib_io.h"

bool packFromZip(pack *px, FILE *fp, uint8_t *buf)
{
	uint8_t xt[MAGNET_XT_LEN];
	uint32_t i, len, nkt;
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

	if (1 != fread(buf, 1, 1, fp)) return false;
	nkt = buf[0];
	if (nkt > MAGNET_KT_COUNT) return false;

	for (i = 0; i < nkt; ++i)
	{
		if (1 != fread(buf, 1, 1, fp)) return false;
		len = buf[0];
		if (!len || len > MAGNET_KT_LEN || len != fread(buf, 1, len, fp)) goto cleanup;
		if (!(kt[i] = (char *) calloc(len + 1, 1))) goto cleanup;
		memcpy(kt[i], buf, len);
	}

	return newPack(px, xt, xl, dn, tr, kt);
cleanup:
	for (i = 0; i < MAGNET_KT_COUNT; ++i) if (kt[i]) free(kt[i]);
	return false;
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
	uint8_t crc[SHA512_LEN], key[SHA512_LEN];
	uint32_t i = 0, nRead, n_packs, n_trans;
	uint64_t n, time;
	pack *packs = NULL;
	tran *trans = NULL;

	nRead = 1 + 2 * SHA512_LEN;
	if (nRead != fread(buf, 1, nRead, fp)) return false;
	if (buf[0] != 'B') return false;
	memcpy(crc, buf + 1, SHA512_LEN);
	memcpy(key, buf + 1 + SHA512_LEN, SHA512_LEN);

	if (18 != fread(buf, 1, 18, fp)) return false;
	u64Unpack(buf, &n);
	u64Unpack(buf + 8, &time);
	n_packs = buf[16];
	n_trans = buf[17];
	if (n_packs) packs = (pack *) calloc(n_packs, sizeof(pack));
	if (n_trans) trans = (tran *) calloc(n_trans, sizeof(tran));

	for (i = 0; i < n_packs; i++) if (!packFromZip(&(packs[i]), fp, buf)) goto cleanup;
	for (i = 0; i < n_trans; i++) if (!tranFromZip(&(trans[i]), fp, buf)) goto cleanup;
	// TODO: run a sha3-512 check to see if crc matches

	return insertBlock(ch, n, time, n_packs, packs, n_trans, trans, crc, key);
cleanup:
	if (packs) free(packs);
	if (trans) free(trans);
	return false;
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

uint32_t importPack(const char *src)
{
	uint8_t xt[MAGNET_XT_LEN];
	uint32_t ret = 0, blen, slen;
	uint64_t xl;
	char buf[BUF4K], dn[MAX_U8 + 1], tr[MAX_U8 + 1];
	char *fio = NULL, *idx, *tok;
	char *kt[MAGNET_KT_COUNT];
	FILE *fp = fopen(src, "r");

	if (!fp || !(blen = getFilesize(fp))) goto cleanup;
	printf("fsize %u\n", blen);
	if (!(fio = (char *) calloc(blen + 1, 1))) goto cleanup;
	if (blen != fread(fio, 1, blen, fp)) goto cleanup;

	for (idx = fio;; ret++)
	{
		if (!(tok = strstr(idx, "size:"))) break;
		if (!(idx = strchr(tok, '\n'))) break;
		slen = idx - tok;
		memcpy(buf, tok, slen);
		buf[slen] = 0;
		printf("%s\n", buf);
		xl = strtol(buf, NULL, 0);

		if (!(tok = strstr(idx, "major:"))) break;
		if (!(idx = strchr(tok, '\n'))) break;
		slen = idx - tok;
		memcpy(buf, tok, slen);
		buf[slen] = 0;
		printf("%s\n", buf);
		//kt[0] = calloc(slen + 1, 1);
		//memcpy(kt, tok, slen);
	}

cleanup:
	if (fp) fclose(fp);
	if (fio) free(fio);
	return ret;
}
