#include <stdlib.h>
#include <string.h>

#include "main_lib.h"
#include "file_io.h"

uint32_t getFilesize(FILE *fp)
{
	uint32_t size;
	if (!fp) return 0;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return size;
}

void printBytes(FILE *fp, uint8_t *data, uint32_t len, const char *suffix)
{
	if (!fp) return;
	for (uint32_t i = 0; i < len; i++) fprintf(fp, "%02x", data[i]);
	if (suffix) fprintf(fp, suffix);
}

void packToText(pack *px, FILE *fp, bool verbose)
{
	bool decompOK;
	char decompTR[MAGNET_TR_LEN];
	const char *kt1, *kt2;
	int i;
	uint8_t kt1u, kt2u;

	if (!px || !fp) return;
	decompOK = decompressTracker(px->tr, decompTR) > 0;
	if (!decompOK) printf("[WARN] failed to decompressTracker\n");

	convertKeyword(px->kt[0], &kt1u, &kt2u);
	kt1 = getKeyword1(kt1u);
	kt2 = getKeyword2(kt1u, kt2u);

	fprintf(fp, "{P\n\tcrc :");
	printBytes(fp, px->crc, SHAKE_LEN, "\n\txt  :");
	printBytes(fp, px->xt, MAGNET_XT_LEN);
	fprintf(fp,
			",\n\tlen : %lu,"
			"\n\tdn  : %s,"
			"\n\ttr  : %s,"
			"\n\tkt  : %s %s\n",
			px->xl, px->dn, decompOK ? decompTR : (char *) (px->tr),
			kt1 ? kt1 : "nil", kt2 ? kt2 : "nil");

	if (verbose)
	{
		printf("\tUT: %s.\n\tST: ", px->ut ? px->ut : "nil");
		if (px->st) for (i = 0; px->st[i]; ++i) printf("%6u ", px->st[i]);
		printf("\n\tKT: %u %u, ", px->kt[0] & MAX_U4, px->kt[0] >> 4);
		for (i = 1; i < 8; ++i) printf("%2u ", px->kt[i]);
		printf("\n");
	}
	fprintf(fp, "}\n");
}

void torDBToText(torDB *td, const char *dest)
{
	FILE *fp = fopen(dest, "w");
	if (!td || !fp) goto cleanup;

	for (uint64_t i = 0; i < td->pak.size(); ++i) packToText(&(td->pak[i]), fp);
cleanup:
	if (fp) fclose(fp);
}

void tranToText(tran *tx, FILE *fp)
{
	if (!tx || !fp) return;
	fprintf(fp, "\t{T"
			"\n\t\tid  : %lu,"
			"\n\t\tsum : %lu.%u,"
			"\n\t\tsrc : ",
			tx->id, tx->deci, tx->frac);
	printBytes(fp, tx->src, ED448_LEN, ",\n\t\tdest: ");
	printBytes(fp, tx->dest, ED448_LEN, ",\n\t\tsig : ");
	printBytes(fp, tx->sig, ED448_SIG_LEN, ",\n\t\t},\n");
}

void blockToText(block *bx, FILE *fp, bool verbose)
{
	if (!bx || !fp) return;
	fprintf(fp, "{B\ncrc:");
	printBytes(fp, bx->crc, SHA3_LEN, "\nkey:");
	printBytes(fp, bx->key, SHA3_LEN);

	fprintf(fp,
			"\n\tn   : %lu,"
			"\n\ttime: %lu,"
			"\n\ttran: %u,\n",
			bx->n, bx->time, bx->n_trans);

	if (verbose) for (uint32_t i = 0; i < bx->n_trans; ++i) tranToText(&(bx->trans[i]), fp);
	fprintf(fp, "},\n");
}

void chainToText(chain *ch, const char *dest)
{
	FILE *fp = fopen(dest, "w");
	if (!ch || !fp) goto cleanup;

	fprintf(fp, "C %lu\n", ch->blk.size());
	for (uint64_t i = 0; i < ch->blk.size(); ++i) blockToText(&(ch->blk[i]), fp);
cleanup:
	if (fp) fclose(fp);
}

bool torDBToZip(torDB *td, const char *dest)
{
	bool ret = false;
	FILE *fp = fopen(dest, "wb");
	uint8_t buf[BUF1K];
	uint32_t j, slen;
	uint64_t i;
	if (!td || !fp) goto cleanup;

	buf[0] = 'C';
	u64Packer(buf + 1, td->pak.size());
	fwrite(buf, 1, 9, fp);

	for (i = 0; i < td->pak.size(); ++i)
	{
		j = 0;
		buf[j++] = 'P';
		memcpy(buf + j, td->pak[i].crc, SHAKE_LEN);
		j += SHAKE_LEN;
		memcpy(buf + j, td->pak[i].xt, MAGNET_XT_LEN);
		j += MAGNET_XT_LEN;
		j += u64Packer(buf + j, td->pak[i].xl);

		slen = strlen(td->pak[i].dn);
		buf[j++] = (uint8_t) slen;
		memcpy(buf + j, td->pak[i].dn, slen);
		j += slen;

		slen = u8len(td->pak[i].tr);
		buf[j++] = (uint8_t) (slen >> 8);
		buf[j++] = (uint8_t) (slen & MAX_U8);
		memcpy(buf + j, td->pak[i].tr, slen);
		j += slen;

		buf[j++] = td->pak[i].kt[0];
		fwrite(buf, 1, j, fp);
	}
	ret = true;
cleanup:
	if (fp) fclose(fp);
	return ret;
}

bool tranToZip(tran *tx, FILE *fp, uint8_t *buf)
{
	uint32_t i = 0;
	if (!tx || !fp || !buf) return false;

	buf[i++] = 'T';
	i += u64Packer(buf + i, tx->id);
	i += u64Packer(buf + i, tx->deci);
	i += u16Packer(buf + i, tx->frac);
	fwrite(buf, 1, i, fp);
	fwrite(tx->src, 1, ED448_LEN, fp);
	fwrite(tx->dest, 1, ED448_LEN, fp);
	fwrite(tx->sig, 1, ED448_SIG_LEN, fp);
	return true;
}

bool chainToZip(chain *ch, const char *dest)
{
	bool ret = false;
	FILE *fp = fopen(dest, "wb");
	uint8_t buf[BUF1K];
	uint32_t j, k;
	uint64_t i;
	if (!ch || !fp) goto cleanup;

	buf[0] = 'C';
	u64Packer(buf + 1, ch->blk.size());
	fwrite(buf, 1, 9, fp);

	for (i = 0; i < ch->blk.size(); ++i)
	{
		j = 0;
		buf[j++] = 'B';
		memcpy(buf + j, ch->blk[i].crc, SHA3_LEN);
		j += SHA3_LEN;
		memcpy(buf + j, ch->blk[i].key, SHA3_LEN);
		j += SHA3_LEN;
		j += u64Packer(buf + j, ch->blk[i].n);
		j += u64Packer(buf + j, ch->blk[i].time);
		buf[j++] = ch->blk[i].n_trans & MAX_U8;
		fwrite(buf, 1, j, fp);

		for (k = 0; k < ch->blk[i].n_trans; ++k) if (!tranToZip(&(ch->blk[i].trans[k]), fp, buf)) goto cleanup;
	}
	ret = true;
cleanup:
	if (fp) fclose(fp);
	return ret;
}
