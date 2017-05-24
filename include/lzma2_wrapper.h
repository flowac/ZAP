#ifndef _LZMA2_WRAPPER_H
#define _LZMA2_WRAPPER_H

#include <string>

#define buffer_cread_size 65536

/* open a file and read raw data from it, then pass this data to
 * compress_data.
 * INPUT:
 * char *in_path - path to the file we want to open
 * char *out_path - path to file we will write compressed data to
 * OUTPUT:
 * 1 - success
 * 0 - failure
 */
int compress_file(const char *in_path, const char *out_path);

/* compress raw data, currently gonna use lzma ill update later to
 * use lzma2.
 * INPUT:
 * unsigned char *input - raw data that we will compress
 * unsigned char *output - destination of compressed data
 * OUTPUT:
 * 1 - success
 * 0 - failure
 * unsigned char *output will hold the compressed data
 */
int compress_data(unsigned char *input, size_t input_len,
                  unsigned char *output, size_t *output_len);

#endif // _LZMA2_WRAPPER_H
