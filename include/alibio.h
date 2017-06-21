#ifndef _ALIBIO_H
#define _ALIBIO_

#include "atype.h"

//No comment
chain *chainExtractor(char *inFile);

/* convert chain to single file without compressing */
void *chainToText_to_file(chain *ch, uint8_t parts);

int text2Chainz(FILE *fp, chain *ch);

/* PROTOTYPE converting file2chainz */
chain *file_2_chainz(FILE *fp);

/* PROTOTYPEget the substring between needle_start and needle_end,
 * allocate memory, copy contents over, return string
 */
char *indexes_of(char *haystack, char *needle_start,
                 char *needle_end);

/* compact the entire chain into x parts, using x threads
 * INPUT:
 * chain *ch - self explanitory
 * uint8_t parts - number of threads to use and
 * also the number of files to split the info into
 */
bool chainCompactor(chain *ch, uint8_t parts = 1);

/* convert struct block to text, each block begins with {B and ends
 * with }.
 * INPUT:
 * block *bx - the block to be printed
 * FILE *fp - file pointer to text file (change this to
 * a stream in the future ?)
 * char *buf - a buffer that will be used to hold data
 * int len - length of the buffer
 */
void blockToText(block *bx, FILE *fp, char *buf, int len);

/* convert a struct tran to to text, each text tran will begin
 * with {t and end with }
 * INPUT:
 * train *tx - the struct tran to be printed
 * FILE *fp - file pointer to text file to write to
 * char *buf - buffer that will be used to put the tran data
 * int len - length of buf
 */
void tranToText(tran *tx, FILE *fp, char *buf, int len);

/* convert a struct pack to text, each text pack will begin
 * with {p and end with }
 * INPUT:
 * pack *pk - struct pack to be printed
 * FILE *fp - file pointer to text file to write to
 * char *buf - buffer that will be used to put the pack data
 * int len - length of the buffer
 */
void packToText(pack *pk, FILE *fp, char *buf, int len);

/* A thread start routing that will convert x number of blocks
 * to text, it will print the blocks from struct
 * threadParams.start to struct threadparams.end
 * INPUT:
 * void *args - usually will be struct threadparams
 */
void *chainToText(void *args);
#endif//_ALIBIO_H
