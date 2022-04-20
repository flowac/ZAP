#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "alib.h"
#include "alibio.h"

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
	fprintf(fp, "\t{P"
			"\n\t\tPlen : %lu,"
			"\n\t\tPdn  : %s,"
			"\n\t\tPxt  : %s,"
			"\n\t\tPtr  : %s,"
			"\n\t\tPinfo: %s,"
			"\n\t\t},\n",
			pk->xl, pk->dn, pk->xt, pk->tr, pk->info);
}

void tranToText(tran *tx, FILE *fp)
{
	fprintf(fp, "\t{T"
			"\n\t\tTtime: %lu,"
			"\n\t\tTid  : %lu,"
			"\n\t\tTsum : %lu,"
			"\n\t\tTsrc : %lu,"
			"\n\t\tTdest: %lu,"
			"\n\t\t},\n",
			tx->time, tx->id, tx->amount, tx->src, tx->dest);
}

void blockToText(block *bx, FILE *fp)
{
	fprintf(fp, "{B\n");
	printBytes(fp, bx->key, 64, "\n");
	printBytes(fp, bx->crc, 64);

	fprintf(fp,
			"\n\tBn   : %lu,"
			"\n\tBgmt : %lu,"
			"\n\tBpack: %u,"
			"\n\tBtran: %u,\n",
			bx->n, bx->time, bx->n_packs, bx->n_trans);

	for (uint32_t i = 0; i < bx->n_packs; ++i) packToText(&(bx->packs[i]), fp);
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

uint32_t charPacker(uint8_t *buf, const char *str, uint32_t len)
{
	char ch;
	uint8_t uch;
	uint32_t i = 0, j;
	buf[i] = len & MAX_U8;

	for (j = 0; j < len; j++)
	{
		ch = str[j];
		if      (ch < 0x20) uch = 0;
		else if (ch < 0x60) uch = ch - 0x20;
		else if (ch < 0x80) uch = ch - 0x40;
		else                uch = 0;

		switch (j % 4)
		{
		case 0: buf[++i] = uch & MAX_U6; break;
		case 1: buf[i] |= (uch & MAX_U2) << 6; buf[++i] = (uch & MAX_U4); break;
		case 2: buf[i] |= (uch & MAX_U4) << 4; buf[++i] = (uch & MAX_U2); break;
		case 3: buf[i] |= (uch & MAX_U6) << 2; break;
		}
	}
	return i + 1;
}

void packToZip(pack *pk, FILE *fp, uint8_t *buf, uint32_t len)
{
	uint32_t i = 0, slen;
	if (!pk || !fp || !buf) return;

	buf[i++] = 'P';
	i += u64Packer(buf + i, pk->xl);

	// TODO: log a message on overflow? fix the length calc?
	if ((slen = strlen(pk->dn)) > (len - i)) return;
	i += charPacker(buf + i, pk->dn, slen);

	if ((slen = strlen(pk->xt)) > (len - i)) return;
	i += charPacker(buf + i, pk->xt, slen);

	if ((slen = strlen(pk->tr)) > (len - i)) return;
	i += charPacker(buf + i, pk->tr, slen);

	if (INFO_LEN > (len - i)) return;
	memcpy(buf + i, pk->info, INFO_LEN);
	i += INFO_LEN;

	fwrite(buf, 1, i, fp);
}

void tranToZip(tran *tx, FILE *fp, uint8_t *buf, uint32_t len)
{
	uint32_t i = 0;
	if (!tx || !fp || !buf) return;

	buf[i++] = 'T';
	i += u64Packer(buf + i, tx->time);
	i += u64Packer(buf + i, tx->id);
	i += u64Packer(buf + i, tx->amount);
	i += u64Packer(buf + i, tx->src);
	i += u64Packer(buf + i, tx->dest);

	fwrite(buf, 1, i, fp);
}

void blockToZip(block *bx, FILE *fp, uint8_t *buf, uint32_t len)
{
	uint32_t i = 0;
	if (!bx || !fp || !buf) return;

	buf[i++] = 'B';
	memcpy(buf + i, bx->key, 64);
	i += 64;
	memcpy(buf + i, bx->crc, 64);
	i += 64;
	i += u64Packer(buf + i, bx->n);
	i += u64Packer(buf + i, bx->time);
	buf[i++] = bx->n_packs & MAX_U8;
	buf[i++] = bx->n_trans & MAX_U8;
	fwrite(buf, 1, i, fp);

	for (i = 0; i < bx->n_packs; i++) packToZip(&(bx->packs[i]), fp, buf, len);
	for (i = 0; i < bx->n_trans; i++) tranToZip(&(bx->trans[i]), fp, buf, len);
}

bool chainToZip(chain *ch, const char *dest)
{
	FILE *fp;
	uint8_t buf[BUF4K];
	if (!ch || !dest || !(fp = fopen(dest, "wb"))) return false;

	memset(buf, 0, BUF4K);
	buf[0] = 'C';
	u64Packer(buf + 1, ch->blk.size());
	fwrite(buf, 1, 9, fp);

	for (uint64_t i = 0; i < ch->blk.size(); ++i) blockToZip(&(ch->blk[i]), fp, buf, BUF4K);

	fclose(fp);
	return true;
}
