#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

void printBytes(FILE *fp, uint8_t *data, uint32_t len)
{
	if (!fp) return;
	for (uint32_t i = 0; i < len; i++) fprintf(fp, "%02x", data[i]);
}

void packToText(pack *pk, FILE *fp, char *buf, int len)
{
	//2 tabs
	if (!pk || !fp || !buf)
		return;
	snprintf(buf, len, "\t{P"
			 "\n\t\tPinfo: %s,"
			 "\n\t\tPlen : %lu,"
			 "\n\t\tPdn  : %s,"
			 "\n\t\tPxt  : %s,"
			 "\n\t\tPtr  : %s,"
			 "\n\tP},\n",
			 pk->info, pk->xl, pk->dn, pk->xt, pk->tr);

	fwrite(buf, 1, strlen(buf), fp);
}

void tranToText(tran *tx, FILE *fp, char *buf, int len)
{
	//2 tabs
	if (!tx || !fp || !buf)
		return;
	snprintf(buf, len, "\t{T"
			 "\n\t\tTtime: %lu,"
			 "\n\t\tTid  : %lu,"
			 "\n\t\tTsum : %lu,"
			 "\n\t\tTsrc : %lu,"
			 "\n\t\tTdest: %lu,"
			 "\n\t},\n",
			 tx->time, tx->id, tx->amount, tx->src, tx->dest);

	fwrite(buf, 1, strlen(buf), fp);
}

void blockToText(block *bx, FILE *fp, char *buf, int len)
{
	//1 tabs
	uint32_t i;
	snprintf(buf, len, "{B"
			 "\n\tBgmt : %lu,"
			 "\n\tBn   : %lu,"
			 "\n\tBkey : %lu,"
			 "\n\tBpack: %u,"
			 "\n\tBtran: %u,\n",
			 bx->time, bx->n, bx->key, bx->n_packs, bx->n_trans);
	
	fwrite(buf, 1, strlen(buf), fp);
	printBytes(fp, bx->crc, 64);

	for (i = 0; i < bx->n_packs; i++) {
		packToText(&(bx->packs[i]), fp, buf, len);
	}
	for (i = 0; i < bx->n_trans; i++) {
		tranToText(&(bx->trans[i]), fp, buf, len);
	}

	strcpy(buf, "B},\n");
	fwrite(buf, 1, strlen(buf), fp);
}

bool chainToText(chain *ch, const char *dest)
{
	char buf[BUF4K];
	FILE *fp;
	if (!ch || !dest || !(fp = fopen(dest, "w"))) return false;

	memset(buf, 0, BUF4K);
	for (uint64_t i = 0; i < ch->n_blk; i++) {
		blockToText(&(ch->blk[i]), fp, buf, BUF4K - 1);
	}

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

uint32_t longPacker(uint8_t *buf, uint64_t data)
{
	for (uint32_t i = 0; i < 8; i++)
	{
		buf[i] = (data >> (i * 8)) & MAX_U8;
	}
	return 8U;
}

void packToZip(pack *pk, FILE *fp, uint8_t *buf, uint32_t len)
{
	uint32_t i, slen;
	if (!pk || !fp || !buf) return;

	buf[0] = 'P';
	memcpy(buf + 1, pk->info, INFO_LEN);
	i = 1 + INFO_LEN;
	i += longPacker(buf + i, pk->xl);

	// log a message? fix the length calc?
	if ((slen = strlen(pk->dn)) > (len - i)) return;
	i += charPacker(buf + i, pk->dn, slen);

	if ((slen = strlen(pk->xt)) > (len - i)) return;
	i += charPacker(buf + i, pk->xt, slen);

	if ((slen = strlen(pk->tr)) > (len - i)) return;
	i += charPacker(buf + i, pk->tr, slen);

	fwrite(buf, 1, i, fp);
}

void tranToZip(tran *tx, FILE *fp, uint8_t *buf, uint32_t len)
{
	uint32_t i;
	if (!tx || !fp || !buf) return;

	buf[0] = 'T';
	i = 1;
	i += longPacker(buf + i, tx->time);
	i += longPacker(buf + i, tx->id);
	i += longPacker(buf + i, tx->amount);
	i += longPacker(buf + i, tx->src);
	i += longPacker(buf + i, tx->dest);

	fwrite(buf, 1, i, fp);
}

void blockToZip(block *bx, FILE *fp, uint8_t *buf, uint32_t len)
{
	uint32_t i;
	if (!bx || !fp || !buf) return;

	buf[0] = 'B';
	i = 1;
	i += longPacker(buf + i, bx->time);
	i += longPacker(buf + i, bx->n);
	memcpy(buf + i, bx->crc, 64);
	i += 64;
	i += longPacker(buf + i, bx->key);
	buf[i++] = bx->n_packs & MAX_U8;
	buf[i++] = bx->n_trans & MAX_U8;
	fwrite(buf, 1, i, fp);

	for (i = 0; i < bx->n_packs; i++) {
		packToZip(&(bx->packs[i]), fp, buf, len);
	}
	for (i = 0; i < bx->n_trans; i++) {
		tranToZip(&(bx->trans[i]), fp, buf, len);
	}
}

bool chainToZip(chain *ch, const char *dest)
{
	FILE *fp;
	uint8_t buf[BUF4K];
	if (!ch || !dest || !(fp = fopen(dest, "wb"))) return false;

	memset(buf, 0, BUF4K);
	buf[0] = 'C';
	longPacker(buf + 1, ch->n_blk);
	fwrite(buf, 1, 9, fp);

	for (uint64_t i = 0; i < ch->n_blk; i++) {
		blockToZip(&(ch->blk[i]), fp, buf, BUF4K);
	}

	fclose(fp);
	return true;
}
