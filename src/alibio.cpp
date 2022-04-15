#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "alib.h"
#include "log.h"
#include "alibio.h"
#include "lzma_wrapper.h"
#include "LzmaEnc.h"

uint32_t getFilesize(FILE *fp)
{
	uint32_t size;
	if (!fp) return 0;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return size;
}

void packToText(pack *pk, FILE *fp, char *buf, int len)
{
	//2 tabs
	if (!pk || !fp || !buf)
		return;
	snprintf(buf, len, "\t{P\
\n\t\tPinfo: %s,\
\n\t\tPdn  : %s,\
\n\t\tPlen : %lld,\
\n\t\tPxt  : %s,\
\n\t\tPtr  : %s,\
\n\tP},\n", pk->info, pk->dn, pk->xl, pk->xt, pk->tr);

	fwrite(buf, 1, strlen(buf), fp);
}

void tranToText(tran *tx, FILE *fp, char *buf, int len)
{
	//2 tabs
	if (!tx || !fp || !buf)
		return;
	snprintf(buf, len, "\t{T\
\n\t\tTtime: %d,\
\n\t\tTid  : %d,\
\n\t\tTsrc : %ld,\
\n\t\tTdest: %ld,\
\n\t\tTsum : %ld,\
\n\t\tTkey : %ld,\
\n\t},\n", tx->time, tx->id, tx->src, tx->dest, tx->amount, tx->key);

	fwrite(buf, 1, strlen(buf), fp);
}

void blockToText(block *bx, FILE *fp, char *buf, int len)
{
	//1 tabs
	uint32_t i;
	snprintf(buf, len, "{B\
\n\tBgmt : %d,\
\n\tBcrc : %d,\
\n\tBpack: %d,\
\n\tBtran: %d,\
\n\tBn   : %d,\
\n\tBkey : %ld,\n", bx->time, bx->crc, bx->n_packs, bx->n_trans, bx->n, bx->key);

	fwrite(buf, 1, strlen(buf), fp);

	for (i = 0; i < bx->n_packs; i++) {
		packToText(&(bx->packs[i]), fp, buf, len);
	}
	for (i = 0; i < bx->n_trans; i++) {
		tranToText(&(bx->trans[i]), fp, buf, len);
	}

	strcpy(buf, "B},\n");
	fwrite(buf, 1, strlen(buf), fp);
}

/*temporary not used
//returns number of bytes read (always higher than actual buf1 strlen)
uint32_t read_struct(FILE *fp, char **buf1, char **buf2, uint32_t *len)
{
    if (!*len)  *len  = MAX_U8;
    if (!*buf1) *buf1 = (char *)malloc(*len);
    if (!*buf2) *buf2 = (char *)malloc(*len), memset(*buf2, 0, *len);
    
    memset(*buf1, 0, *len);
    memcpy(*buf1, *buf2, *len);
    memset(*buf2, 0, *len);
    
    char *buf3;
    uint32_t lb1 = strlen(*buf1), red = 0;//strlen of buf1, bytes read
    
    while ( !(buf3 = strchr(*buf2, '}')) ) {//end brace not found
        memset(*buf2, 0, *len);
        if ( !(red = fread(*buf2, MAX_U6, 1, fp)) ) {
            return lb1;//EOF
        }
        
        if (lb1 + MAX_U6 > *len) {
            //increase buffer size to prevent overflow
            *len += MAX_U8;
            assert(*len < MAX_U16);
            
            *buf1 = (char *)realloc(*buf1, *len);
            *buf2 = (char *)realloc(*buf2, *len);
        }
        
        memcpy(*buf1 + lb1, *buf2, red);
        lb1 += red;
    }
    
    // previous size + 1 +  buf1 + offset of '}' in buf2
    buf3 = lb1 - red + 1 + *buf1 - *buf2 + buf3;//location of the ending brace
    strcpy(*buf2, buf3);
    *buf1[buf3 - *buf1 + 1] = 0;//terminate the brace
    
    return lb1;
//boost::regex_search(buf,eReg);
//buf2 = boost::regex_replace(buf2, convertReg2, pad, boost::format_first_only);
}
*/

//To be DEPRECIATED
char *indexes_of(char *haystack, const char *needle_start,
				 const char *needle_end)
{
	char *ptr_start = strstr(haystack, needle_start);
	char *ptr_end = strstr(haystack, needle_end);

	if (!(ptr_start && ptr_end))
		return NULL;

	int len = ptr_end - ptr_start + 2;
	char *dest = (char *) malloc(sizeof(char) *len + 1);
	dest[len] = '\0';
	memcpy(dest, ptr_start + 2, len);
	return dest;
}

bool text2Pac(pack *px, FILE *fp)
{
	bool ret = 1;
	char s[MAX_U8 + 1], *dn = NULL, *xt = NULL, *p_xl = NULL, *tr = NULL;
	uint64_t xl = 0;

	while (fgets(s, MAX_U8, fp) != NULL) {
		int len = 0;
		char *data = strstr(s, (char *) "P");
		if (data) {
			len = strlen(data);
		}
		if (len > 1) {
			switch (data[1]) {
			case 'i':			// pack->info
				break;
			case 'd':			// pack->dn
				dn = indexes_of(data, ": ", ",");
				break;
			case 'l':			// pack->xl
				p_xl = indexes_of(data, ": ", ",");
				xl = atof(p_xl);
				break;
			case 'x':			// pack->xt
				xt = indexes_of(data, ": ", ",");
				break;
			case 't':			// pack->tr
				tr = indexes_of(data, ": ", ",");
				break;
			case '}':			// end of pack
				ret = newPack(px, dn, xl, xt, tr);
				if (dn)
					free(dn);
				if (p_xl)
					free(p_xl);
				if (xt)
					free(xt);
				if (tr)
					free(tr);
			default:
				break;
			}
		}
	}
	return ret;
}

tran *text2Tran(FILE *fp)
{
	return NULL;
}

void text2Block(FILE *fp, block *bx)
{
	char s[MAX_U8 + 1];
	pack *packs = 0, px;
	char *tmp;
	uint32_t time = 0;
	uint64_t crc = 0;
	uint64_t n_pack = 0;
	uint64_t n_tran = 0;
	uint64_t n = 0;
	uint64_t key = 0;

	while (fgets(s, MAX_U8, fp) != NULL) {
		if (strstr(s, (char *) "{P") != NULL) {
			if (!text2Pac(&px, fp)) {
				n_pack++;
				if (n_pack > MAX_U16) {
					deletePack(&px);
					log_msg_custom("nPack limit reached\n");
					break;
				}
				packs = (pack *) realloc(packs, sizeof(pack) * n_pack);
				packs[n_pack - 1] = px;
			}
		} else {
			int len = 0;
			char *data = strstr(s, (char *) "B");
			if (data)
				len = strlen(data);
			if (len > 1) {
				switch (data[1]) {
				case 'g':		// block->time
					tmp = indexes_of(data, ": ", ",");
					time = atol(tmp);
					if (tmp)
						free(tmp);
					break;
				case 'c':		// block->crc
					tmp = indexes_of(data, ": ", ",");
					crc = atol(tmp);
					if (tmp)
						free(tmp);
					break;
				case 'p':		// block->nPack
					/* n_pack is being counted in the other
					 * part of the loop
					 */
					//tmp = indexes_of(data, ": ", ",");
					//n_pack = atol(tmp);
					//if (tmp) free(tmp);
					break;
				case 't':		// block->nTran
					tmp = indexes_of(data, ": ", ",");
					n_tran = atol(tmp);
					if (tmp)
						free(tmp);
					break;
				case 'n':		// block->n
					tmp = indexes_of(data, ": ", ",");
					n = atol(tmp);
					if (tmp)
						free(tmp);
					break;
				case 'k':		// block->key
					tmp = indexes_of(data, ": ", ",");
					key = strtoll(tmp, NULL, 10);
					if (tmp)
						free(tmp);
					break;
				case '}':
					newBlock(bx, time, n, key, &n_pack, &packs);
					break;
				default:
					break;
				}
			}
		}
	}
}

int text2Chainz(FILE *fp, chain *ch)
{
	block bx;
	char s[MAX_U8 + 1];
	if (ch == NULL)
		return 0;

	/* Parse the text searching for the start of a block
	 *
	 */
	while (fgets(s, MAX_U8, fp) != NULL) {
		if (strstr(s, (char *) "{B") != NULL) {
			text2Block(fp, &bx);
			if (!insertBlock(&bx, ch))
				log_msg_custom("Failed to insert block");
		} else {
			int len = 0;
			char *data = strstr(s, (char *) "C");
			if (data)
				len = strlen(data);
			if (len > 1) {
				switch (data[1]) {
				case 't':
					// Ctime
					break;
					// Csize
				case 's':
					break;
				default:
					break;
				}
			}
		}
	}
	return 1;
}

chain *file_2_chainz(FILE *fp)
{
	chain *ch = newChain();
	if (!text2Chainz(fp, ch))
		log_msg_default;
	return NULL;
}

//  return 1 for success, 0 for failure
bool chainCompactor(chain *ch)
{
	FILE *fp = fopen("temp.file", "w");
	const int b4k = 4096;
	char buf4k[b4k + 1];
	for (uint64_t i = 0; i < ch->n_blk; i++) {
		blockToText(&(ch->blk[i]), fp, buf4k, b4k);
	}

	fclose(fp);
	return 1;
}

//  return 1 for success, 0 for failure
/* Right now dont worry about compressing/decompressing
 */
chain *chain_extractor(const char *inFile, uint8_t parts)
{
	char buffy[69];
	chain *ch = newChain();
	FILE *fp = NULL;
	for (int i = 1; i <= parts; i++) {
		sprintf(buffy, "temp%d.file", i);
		fp = fopen(buffy, "r");
		if (fp == NULL) {
			log_msg_default;
			continue;
		}
		text2Chainz(fp, ch);
	}
	return ch;
}
