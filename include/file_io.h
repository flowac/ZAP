/**
 * @file file_io.h
 * @brief Functions that deal with i/o of the chain
 */
#ifndef _ALIB_IO_H
#define _ALIB_IO_H

#include <stdio.h>
#include "types.h"

/**
 * @brief Get remaining size of the file
 */
uint32_t getFilesize(FILE *fp);

/**
 * @brief Print the raw data to a stream as hex
 */
void printBytes(FILE *fp,
				uint8_t *data,
				uint32_t len,
				const char *suffix = NULL);

/**
 * @brief convert the entire chain into a file
 */
bool chainToText(chain *ch,        //!< Chain to write
				 const char *block_file,
				 const char *pack_file);

/**
 * @brief compress the entire chain
 */
bool chainToZip(chain *ch,        //!< Chain to be compressed
				const char *block_file,
				const char *pack_file);

/**
 * @brief extract the entire chain
 */
bool chainFromZip(chain *ch,      //!< Chain to be extracted 
				  const char *block_file,
				  const char *pack_file);

/**
 * @brief process new torrents and append to the queue
 */
uint32_t importPack(chain *ch, const char *src);

#endif //_ALIB_IO_H
