#include <stdlib.h>
#include <string.h>

#include "main_lib.h"
#include "file_io.h"

bool packFromZip(chain *ch, const char *src, uint8_t *buf)
{
	bool ret = false;
	char dn[MAGNET_DN_LEN + 1];
	char *kt[MAGNET_KT_COUNT] = {NULL, NULL, NULL, NULL, NULL};
	FILE *fp = fopen(src, "rb");
	uint8_t xt[MAGNET_XT_LEN];
	uint8_t tr[MAGNET_TR_LEN];
	uint32_t j, len, nkt;
	uint64_t i, nPacks, xl;
	if (!fp || !buf) goto cleanup;

	if (9 != fread(buf, 1, 9, fp)) goto cleanup;
	if (buf[0] != 'C') goto cleanup;
	u64Unpack(buf + 1, &nPacks);

	for (i = 0; i < nPacks; ++i)
	{
		len = 9 + MAGNET_XT_LEN;
		if (len != fread(buf, 1, len, fp)) goto cleanup;
		if (buf[0] != 'P') goto cleanup;
		memcpy(xt, buf + 1, MAGNET_XT_LEN);
		u64Unpack(buf + 1 + MAGNET_XT_LEN, &xl);

		if (1 != fread(buf, 1, 1, fp)) goto cleanup;
		len = buf[0];
		if (len > MAGNET_DN_LEN || len != fread(buf, 1, len, fp)) goto cleanup;
		memcpy(dn, buf, len);
		dn[len] = 0;

		if (2 != fread(buf, 1, 2, fp)) goto cleanup;
		len = buf[0] << 8;
		len += buf[1];
		if (len >= MAGNET_TR_LEN || len != fread(buf, 1, len, fp)) goto cleanup;
		memcpy(tr, buf, len);
		tr[len] = 0;

		if (1 != fread(buf, 1, 1, fp)) goto cleanup;
		nkt = buf[0];
		if (nkt > MAGNET_KT_COUNT) goto cleanup;

		for (j = 0; j < nkt; ++j)
		{
			if (1 != fread(buf, 1, 1, fp)) goto cleanup;
			len = buf[0];
			if (!len || len > MAGNET_KT_LEN || len != fread(buf, 1, len, fp)) goto cleanup;
			if (!(kt[j] = (char *) calloc(len + 1, 1))) goto cleanup;
			memcpy(kt[j], buf, len);
			kt[j][len] = 0;
		}

		if ((ret = newPack(ch, xt, xl, dn, tr, kt))) for (j = 0; j < nkt; ++j) kt[j] = NULL;
		else goto cleanup;
	}
cleanup:
	if (fp) fclose(fp);
	for (j = 0; j < MAGNET_KT_COUNT; ++j) if (kt[j]) free(kt[j]);
	return ret;
}

bool tranFromZip(tran *tx, FILE *fp, uint8_t *buf)
{
	uint8_t  src[ED448_LEN];
	uint8_t  dest[ED448_LEN];
	uint8_t  sig[ED448_SIG_LEN];
	uint16_t frac;
	uint64_t id, deci;
	if (!fp || !buf) return false;

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

	return newTran(tx, id, deci, frac, src, dest, sig);
}

bool blockFromZip(chain *ch, const char *src, uint8_t *buf)
{
	bool ret = false;
	FILE *fp = fopen(src, "rb");;
	tran *trans = NULL;
	uint8_t crc[SHA3_LEN], key[SHA3_LEN];
	uint32_t j, nRead;
	uint64_t nBlocks, i, n, nTrans, time;
	if (!ch || !fp || !buf) goto cleanup;

	if (9 != fread(buf, 1, 9, fp)) goto cleanup;
	if (buf[0] != 'C') goto cleanup;
	u64Unpack(buf + 1, &nBlocks);

	for (i = 0; i < nBlocks; ++i)
	{
		nRead = 1 + 2 * SHA3_LEN;
		if (nRead != fread(buf, 1, nRead, fp)) goto cleanup;
		if (buf[0] != 'B') goto cleanup;
		memcpy(crc, buf + 1, SHA3_LEN);
		memcpy(key, buf + 1 + SHA3_LEN, SHA3_LEN);

		if (17 != fread(buf, 1, 17, fp)) goto cleanup;
		u64Unpack(buf, &n);
		u64Unpack(buf + 8, &time);
		if ((nTrans = buf[16]))
		{
			trans = (tran *) calloc(nTrans, sizeof(tran));
			for (j = 0; j < nTrans; ++j) if (!tranFromZip(&(trans[j]), fp, buf)) goto cleanup;
		}

		if ((ret = insertBlock(ch, n, time, nTrans, trans, crc, key))) trans = NULL;
		else goto cleanup;
	}
cleanup:
	if (fp) fclose(fp);
	if (trans) free(trans);
	return ret;
}

bool chainFromZip(chain *ch, const char *block_file, const char *pack_file)
{
	uint8_t buf[BUF1K];
	if (!ch || !block_file || !pack_file) return false;

	if (!blockFromZip(ch, block_file, buf) || !packFromZip(ch, pack_file, buf))
	{
		deleteChain(ch);
		return false;
	}

	return true;
}

uint32_t importPack(chain *ch, const char *src)
{
	char dn[MAGNET_DN_LEN];
	char *fio = NULL, *idx, *tok;
	char *kt[MAGNET_KT_COUNT] = {NULL, NULL, NULL, NULL, NULL};
	FILE *fp = fopen(src, "r");
	uint8_t xt[MAGNET_XT_LEN];
	uint8_t tr[MAGNET_TR_LEN];
	uint32_t ret = 0, blen, slen;
	uint64_t xl;

	if (!ch || !fp || !(blen = getFilesize(fp))) goto cleanup;
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

		if (!newPack(ch, xt, xl, dn, tr, kt)) break;
		for (int i = 0; i < MAGNET_KT_COUNT; ++i) kt[i] = NULL;
	}

cleanup:
	for (int i = 0; i < MAGNET_KT_COUNT; ++i) if (kt[i]) free(kt[i]);
	if (fp) fclose(fp);
	if (fio) free(fio);
	return ret;
}
