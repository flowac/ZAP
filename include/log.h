/* GPL stuff here idk what to put @flowingwater ill let you
 * handle this 
 */
#ifndef _LOG_H
#define _LOG_H

#include <fstream>
#include "C/7zTypes.h"

/* This function will append to the log file
 * INPUT:
 * char *msg - message to be printed, can be formatted with %s etc
 * RETURN:
 * 0 - failure
 * 1 - success
 */
int log_msg(char const *msg, ...);

/* Get the size of a file
 * INPUT:
 * FILE *fd - file descriptor pointing to file
 * OUTPUT:
 * size of file (long value)
 */
long get_file_size_c(FILE *fd);

/* This function will return the size of the file specified
 * by f_stream
 * INPUT:
 * std::ofstream f_stream - a filstream to the file
 * RETURN:
 * Size of file on success
 * 0 on failure
 */
UInt64 get_file_size(std::ofstream f_stream);

#endif // _LOG_H
