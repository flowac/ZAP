/**
 * @file file_io.h
 * @brief Functions that deal with i/o of the chain
 */
#ifndef _ALIB_IO_H
#define _ALIB_IO_H

#include <cstdio>
#include "types.h"

uint32_t getFilesize(FILE *fp);
void printBytes(FILE *fp, uint8_t *data, uint32_t len, const char *suffix = NULL);

void chainToText (chain *ch, const char *dest);
bool chainToZip  (chain *ch, const char *dest);
bool chainFromZip(chain *ch, const char *src);
void torDBToText (torDB *td, const char *dest);
bool torDBToZip  (torDB *td, const char *dest);
bool torDBFromZip(torDB *td, const char *src);
bool torDBFromTxt(torDB *td, const char *src);

#endif //_ALIB_IO_H
