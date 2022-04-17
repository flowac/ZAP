/**
 * @file alibio.h
 * @brief Functions that deal with i/o of the chain
 */
#ifndef _ALIBIO_H
#define _ALIBIO_

#include "atype.h"
#include <stdio.h>

/**
 * @brief Get remaining size of the file
 */
uint32_t getFilesize(FILE *fp);

/**
 * @brief Print the raw data to a stream as hex
 */
void printBytes(FILE *fp,
				uint8_t *data,
				uint32_t len);

/**
 * @brief convert the entire chain into a file
 * 
 */
bool chainToText(chain *ch,        //!< Chain to write
				 const char *dest);//!< Destination file

/**
 * @brief compress the entire chain
 * 
 */
bool chainToZip(chain *ch,        //!< Chain to be compressed
				const char *dest);//!< Destination file

#endif //_ALIBIO_H
