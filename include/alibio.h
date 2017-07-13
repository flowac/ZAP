/**
 * @file alibio.h
 * @brief Functions that deal with i/o of the chain
 */
#ifndef _ALIBIO_H
#define _ALIBIO_

#include "atype.h"

//No comment
chain *chain_extractor(const char *inFile, uint8_t parts);

/**
 * @brief Convert chain to single file without compressing
 *
 * ChainToText writes to file and compresses this function will not.
 * This fn is probably just a temp addition will be removed later
 */
void *chainToText_to_file(chain *ch, //!< chain to be printed
                          uint8_t parts //!< number of files to split into
                          );

/**
 * @brief Read from a file and convert to a chain struct
 */
int text2Chainz(FILE *fp, //!< FP to the file to read in
                chain *ch /**< Destination of the chain, must be a
                             valid pointer*/
                );

/**
 * @brief PROTOTYPE converting file2chainz
 */
chain *file_2_chainz(FILE *fp);

/**
 * @brief PROTOTYPE get a substring
 *
 * Find the substring between needle_start and needle_end,
 * allocate memory, copy contents over, return string
 */
char *indexes_of(char       *haystack, //!< The string to search through
                 const char *needle_start, //!< Starting needle
                 const char *needle_end //!< Ending needle
                 );

/**
 * @brief compact the entire chain into x parts, using x threads
 * 
 */
bool chainCompactor(chain *ch, //!< Chain to be compacted
                    uint8_t parts = 1 /**< Number of threads to use 
                                         also the number of files to
                                         split the info into */
                    );

/**
 * @brief convert struct block to text
 *
 * Each block begins with {B and ends with }.
 * 
 * @param fp - file pointer to text file (change this to
 * a stream in the future ?)
 */
void blockToText(block *bx, //!< The block to be printed
                 FILE *fp, //!< File pointer to destination
                 char *buf, //!< Buffer to hold data
                 int len //!< Length of the buffer
                 );

/**
 * @brief convert a struct tran to to text
 *
 * Each text tran will begin with {t and end with }
 * INPUT:
 * train *tx - the struct tran to be printed
 * FILE *fp - file pointer to text file to write to
 * char *buf - buffer that will be used to put the tran data
 * int len - length of buf
 */
void tranToText(tran *tx, //!<
                FILE *fp, //!<
                char *buf, //!<
                int len //!<
                );

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
