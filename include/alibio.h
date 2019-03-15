/**
 * @file alibio.h
 * @brief Functions that deal with i/o of the chain
 */
#ifndef _ALIBIO_H
#define _ALIBIO_

#include "atype.h"

/**
 * @brief Extract a chain from a text file(s)
 *
 * Currently doesnt support decompression, not that much effort to
 * add that in though.
 *
 * This function will take a string argument that will be the base for
 * the files, i.e temp\%d.file, it will substitute parts in to make
 * temp1.file, temp2.file etc... Should improve this in the future.
 */
chain *chain_extractor(const char *inFile,//!< String to filename (not working rn)
                       uint8_t parts);    //!< Number of files

/**
 * @brief Convert chain to single file without compressing
 *
 * ChainToText writes to file and compresses this function will not.
 * This fn is probably just a temp addition will be removed later
 */
void *chainToText_to_file(chain *ch,    //!< chain to be printed
                          uint8_t parts);//!< number of files to split into

/**
 * @brief Read from a file and convert to a chain struct
 */
int text2Chainz(FILE *fp,//!< FP to the file to read in
                chain *ch);//!< Destination

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
char *indexes_of(char *haystack,          //!< The string to search through
                 const char *needle_start,//!< Starting needle
                 const char *needle_end); //!< Ending needle

/**
 * @brief compact the entire chain into x parts, using x threads
 * 
 */
bool chainCompactor(chain *ch,        //!< Chain to be compacted
                    uint8_t parts = 1);//!< Number of threads to use 

/**
 * @brief convert struct block to text
 *
 * Each block begins with {B and ends with }.
 * 
 * @param fp - file pointer to text file (change this to
 * a stream in the future ?)
 */
void blockToText(block *bx,//!< The block to be printed
                 FILE *fp, //!< File pointer to destination
                 char *buf,//!< Buffer to hold data
                 int len); //!< Length of the buffer

/**
 * @brief convert a struct tran to to text
 *
 * Each text tran will begin with {t and end with }
 */
void tranToText(tran *tx, //!< Struct tran to be printed
                FILE *fp, //!< Fp to destination
                char *buf,//!< Buffer used to print data
                int len); //!< Length of buf

/**
 * @brief Convert a struct pack to text
 *
 * Each text pack will begin with {p and end with }
 */
void packToText(pack *pk, //!< Struct pack to be printed
                FILE *fp, //!< Fp to destination
                char *buf,//!< Buffer used to print data
                int len); //!< Length of buffer

/**
 * @brief A thread start routine
 *
 * This fn will convert x number of blocks to text, it will
 * print the blocks from struct threadParams.start to struct
 * threadparams.end
 */
void *chainToText(void *args);//!< Arguments to pass the thread fn
#endif //_ALIBIO_H

