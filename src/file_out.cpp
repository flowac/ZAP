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

void packToText(pack *pk, FILE *fp)
{
	char decompTR[MAGNET_TR_LEN];
	bool decompOK = decompressTracker(pk->tr, decompTR) > 0;
	if (!decompOK) printf("WARNING: failed to decompressTracker\n");

	fprintf(fp, "\t{P\n\t\t");
	printBytes(fp, pk->xt, 20);
	fprintf(fp, ","
			"\n\t\tlen : %lu,"
			"\n\t\tdn  : %s,"
			"\n\t\ttr  : %s,"
			"\n\t\tkt  : ",
			pk->xl, pk->dn, decompOK ? decompTR : (char *) (pk->tr));
	for (uint32_t i = 0; i < MAGNET_KT_COUNT; ++i)
	{
		if (!pk->kt[i] || !pk->kt[i][0]) break;
		fprintf(fp, "%s ", pk->kt[i]);
	}
	fprintf(fp, "\n\t\t},\n");
}

void tranToText(tran *tx, FILE *fp)
{
	fprintf(fp, "\t{T"
			"\n\t\tid  : %lu,"
			"\n\t\tsum : %lu.%u,"
			"\n\t\tsrc : ",
			tx->id, tx->deci, tx->frac);
	printBytes(fp, tx->src, ED448_LEN, ",\n\t\tdest: ");
	printBytes(fp, tx->dest, ED448_LEN, ",\n\t\tsig : ");
	printBytes(fp, tx->sig, ED448_SIG_LEN, ",\n\t\t},\n");
}

void blockToText(block *bx, FILE *fp)
{
	fprintf(fp, "{B\n");
	printBytes(fp, bx->crc, SHA3_LEN, "\n");
	printBytes(fp, bx->key, SHA3_LEN);

	fprintf(fp,
			"\n\tn   : %lu,"
			"\n\tgmt : %lu,"
			"\n\ttran: %u,\n",
			bx->n, bx->time, bx->n_trans);

	for (uint32_t i = 0; i < bx->n_trans; ++i) tranToText(&(bx->trans[i]), fp);

	fprintf(fp, "},\n");
}

bool chainToText(chain *ch, const char *dest)
{
	FILE *fp;
	if (!ch || !dest || !(fp = fopen(dest, "w"))) return false;

	for (uint64_t i = 0; i < ch->blk.size(); ++i) blockToText(&(ch->blk[i]), fp);

	fclose(fp);
	return true;
}

void packToZip(pack *pk, FILE *fp, uint8_t *buf)
{
	uint32_t i = 0, j, slen, ktPos;
	if (!pk || !fp || !buf) return;

	buf[i++] = 'P';
	memcpy(buf + i, pk->xt, MAGNET_XT_LEN);
	i += MAGNET_XT_LEN;
	i += u64Packer(buf + i, pk->xl);

	slen = strlen(pk->dn);
	buf[i++] = (uint8_t) slen;
	memcpy(buf + i, pk->dn, slen);
	i += slen;

	slen = u8len(pk->tr);
	buf[i++] = (uint8_t) (slen >> 8);
	buf[i++] = (uint8_t) (slen & MAX_U8);
	memcpy(buf + i, pk->tr, slen);
	i += slen;

	ktPos = i;
	i++;
	for (j = 0; j < MAGNET_KT_COUNT; ++j)
	{
		if (!pk->kt[j] || !pk->kt[j][0]) break;
		slen = strlen(pk->kt[j]);
		buf[i++] = (uint8_t) slen;
		memcpy(buf + i, pk->kt[j], slen);
		i += slen;
	}
	buf[ktPos] = j;
	fwrite(buf, 1, i, fp);
}

void tranToZip(tran *tx, FILE *fp, uint8_t *buf)
{
	uint32_t i = 0;
	if (!tx || !fp || !buf) return;

	buf[i++] = 'T';
	i += u64Packer(buf + i, tx->id);
	i += u64Packer(buf + i, tx->deci);
	i += u16Packer(buf + i, tx->frac);
	fwrite(buf, 1, i, fp);
	fwrite(tx->src, 1, ED448_LEN, fp);
	fwrite(tx->dest, 1, ED448_LEN, fp);
	fwrite(tx->sig, 1, ED448_SIG_LEN, fp);
}

void blockToZip(block *bx, FILE *fp, uint8_t *buf)
{
	uint32_t i = 0;
	if (!bx || !fp || !buf) return;

	buf[i++] = 'B';
	memcpy(buf + i, bx->crc, SHA3_LEN);
	i += SHA3_LEN;
	memcpy(buf + i, bx->key, SHA3_LEN);
	i += SHA3_LEN;
	i += u64Packer(buf + i, bx->n);
	i += u64Packer(buf + i, bx->time);
	buf[i++] = bx->n_trans & MAX_U8;
	fwrite(buf, 1, i, fp);

	for (i = 0; i < bx->n_trans; i++) tranToZip(&(bx->trans[i]), fp, buf);
}

bool chainToZip(chain *ch, const char *dest)
{
	FILE *fp;
	uint8_t buf[BUF1K];
	if (!ch || !dest || !(fp = fopen(dest, "wb"))) return false;

	memset(buf, 0, BUF1K);
	buf[0] = 'C';
	u64Packer(buf + 1, ch->blk.size());
	fwrite(buf, 1, 9, fp);

	for (uint64_t i = 0; i < ch->blk.size(); ++i) blockToZip(&(ch->blk[i]), fp, buf);

	fclose(fp);
	return true;
}
