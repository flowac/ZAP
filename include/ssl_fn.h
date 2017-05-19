#ifndef _SSL_FN_H
#define _SSL_FN_H

/* String buffer size when reading from file */
#define file_input_buff_size 512

/* Create the sha1sum of the file pointed to by dst
 * DESCRIPTION:
 * Take an absolute path and calculate the sha1sum of this path
 * Gonna need something like this to make the xt URN
 * INPUT:
 * char *dst - string with an absolute path
 * RETURN:
 * NULL - failure
 */
unsigned char *create_sha1sum(const char *dst);

#endif //_SS_FN_H
