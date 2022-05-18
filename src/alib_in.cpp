#include <stdlib.h>
#include <string.h>

#include "alib.h"
#include "alib_io.h"

bool packFromZip(pack *px, FILE *fp, uint8_t *buf)
{
	uint8_t xt[MAGNET_XT_LEN];
	uint8_t tr[MAGNET_TR_LEN];
	uint32_t i, len, nkt;
	uint64_t xl;
	char dn[MAGNET_DN_LEN + 1];
	char *kt[MAGNET_KT_COUNT] = {NULL, NULL, NULL, NULL, NULL};
	if (!px || !fp || !buf) return false;

	len = 9 + MAGNET_XT_LEN;
	if (len != fread(buf, 1, len, fp)) return false;
	if (buf[0] != 'P') return false;
	memcpy(xt, buf + 1, MAGNET_XT_LEN);
	u64Unpack(buf + 1 + MAGNET_XT_LEN, &xl);

	if (1 != fread(buf, 1, 1, fp)) return false;
	len = buf[0];
	if (len > MAGNET_DN_LEN || len != fread(buf, 1, len, fp)) return false;
	memcpy(dn, buf, len);
	dn[len] = 0;

	if (2 != fread(buf, 1, 2, fp)) return false;
	len = buf[0] << 8;
	len += buf[1];
	if (len >= MAGNET_TR_LEN || len != fread(buf, 1, len, fp)) return false;
	memcpy(tr, buf, len);
	tr[len] = 0;

	if (1 != fread(buf, 1, 1, fp)) return false;
	nkt = buf[0];
	if (nkt > MAGNET_KT_COUNT) return false;

	for (i = 0; i < nkt; ++i)
	{
		if (1 != fread(buf, 1, 1, fp)) goto cleanup;
		len = buf[0];
		if (!len || len > MAGNET_KT_LEN || len != fread(buf, 1, len, fp)) goto cleanup;
		if (!(kt[i] = (char *) calloc(len + 1, 1))) goto cleanup;
		memcpy(kt[i], buf, len);
		kt[i][len] = 0;
	}

	return newPack(px, xt, xl, dn, tr, kt);
cleanup:
	for (i = 0; i < MAGNET_KT_COUNT; ++i) if (kt[i]) free(kt[i]);
	return false;
}

bool tranFromZip(tran *tx, FILE *fp, uint8_t *buf)
{
	uint8_t  src[ED448_LEN];
	uint8_t  dest[ED448_LEN];
	uint8_t  sig[ED448_SIG_LEN];
	uint16_t frac;
	uint64_t id, deci;
	if (!tx || !fp || !buf) return false;

	if (19 != fread(buf, 1, 19, fp)) return false;
	if (buf[0] != 'T') return false;
	u64Unpack(buf + 1,  &id);
	u64Unpack(buf + 9,  &deci);
	u16Unpack(buf + 17, &frac);

	if (ED448_LEN != fread(buf, 1, ED448_LEN, fp)) return false;
	memcpy(src, buf, ED448_LEN);
	if (ED448_LEN != fread(buf, 1, ED448_LEN, fp)) return false;
	memcpy(dest, buf, ED448_LEN);
	if (ED448_SIG_LEN != fread(buf, 1, ED448_SIG_LEN, fp)) return false;
	memcpy(sig, buf, ED448_SIG_LEN);

	return newTran(tx, id, frac, deci, src, dest, sig);
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

	return insertBlock(ch, n, time, n_packs, packs, n_trans, trans, crc, key);
cleanup:
	if (packs) free(packs);
	if (trans) free(trans);
	return false;
}

bool chainFromZip(chain *ch, const char *dest)
{
	bool ret = false;
	FILE *fp = NULL;
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
	char dn[MAGNET_DN_LEN];
	char *fio = NULL, *idx, *tok;
	char *kt[MAGNET_KT_COUNT] = {NULL, NULL, NULL, NULL, NULL};
	FILE *fp = fopen(src, "r");
	uint8_t xt[MAGNET_XT_LEN];
	uint8_t tr[MAGNET_TR_LEN];
	uint32_t ret = 0, blen, slen;
	uint64_t xl;
	pack px;

	if (!fp || !(blen = getFilesize(fp))) goto cleanup;
	if (!(fio = (char *) calloc(blen + 1, 1))) goto cleanup;
	if (blen != fread(fio, 1, blen, fp)) goto cleanup;
	fio[blen] = 0;

	for (idx = fio;; ret++)
	{
		xl = 0;
		dn[0] = 0;
		tr[0] = 0;
		if ((tok = strstr(idx, "size:")))
		{
			char buf64[BUF64];
		    tok += 5;
			if (!(idx = strchr(tok, '\n'))) break;
			if ((slen = idx - tok) > 19 || tok >= idx) break; // base 10 uin64_t max length
			memcpy(buf64, tok, slen);
			buf64[slen] = 0;
			xl = strtol(buf64, NULL, 0);
		}

		if ((tok = strstr(idx, "major:")))
		{
		    tok += 6;
			if (!(idx = strchr(tok, '\n'))) break;
			if ((slen = idx - tok) >= MAGNET_KT_LEN || tok >= idx) break;
			kt[0] = (char *) calloc(slen + 1, 1);
			memcpy(kt[0], tok, slen);
		}

		if ((tok = strstr(idx, "minor:")))
		{
		    tok += 6;
			if (!(idx = strchr(tok, '\n'))) break;
			if ((slen = idx - tok) >= MAGNET_KT_LEN || tok >= idx) break;
			kt[1] = (char *) calloc(slen + 1, 1);
			memcpy(kt[1], tok, slen);
		}

		if ((tok = strstr(idx, "name:")))
		{
			tok += 5;
			if (!(idx = strchr(tok, '\n'))) break;
			if ((slen = idx - tok) >= MAGNET_DN_LEN || tok >= idx) break;
			memcpy(dn, tok, slen);
			dn[slen] = 0;
		}
		else break;

		if ((tok = strstr(idx, "magnet:")))
		{
			if (!(tok = strstr(idx, "xt="))) break;
			if (strncmp(tok + 3, "urn:btih:", 9) != 0) break;
			tok += 12;
			if (!(idx = strchr(tok, '&'))) break;
			if ((slen = idx - tok) != MAGNET_XT_LEN * 2 || tok >= idx) break;

			char buf3[3] = {0, 0, 0};
			for (uint32_t i = 0; i < MAGNET_XT_LEN; ++i)
			{
				memcpy(buf3, tok + (i << 1), 2);
				xt[i] = (uint8_t) strtol(buf3, NULL, 16);
			}

			if (!(tok = strstr(idx, "tr="))) break;
			tok += 3;
			if (!(idx = strstr(tok, "\nxdi"))) break;
			if ((slen = idx - tok) >= MAGNET_TR_LEN || tok >= idx) break;
			memcpy(tr, tok, slen);
			tr[slen] = 0;
		}
		else break;

		if (newPack(&px, xt, xl, dn, tr, kt)) enqueuePack(&px);
		else break;

		for (int i = 0; i < MAGNET_KT_COUNT; ++i) kt[i] = NULL;
	}

cleanup:
	for (int i = 0; i < MAGNET_KT_COUNT; ++i) if (kt[i]) free(kt[i]);
	if (fp) fclose(fp);
	if (fio) free(fio);
	return ret;
}
