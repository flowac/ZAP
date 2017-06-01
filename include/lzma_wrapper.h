#ifndef _LZMA2_WRAPPER_H
#define _LZMA2_WRAPPER_H

#include <string>
#include "C/7zTypes.h"
#include "C/LzmaEnc.h"

#define buffer_cread_size 65536 // 1 < 16

/* default values that we are using for the prop
 * in the lzma library, look at lzmalib.h for more info
 * on what each value does, see our definition in alib.cpp
 */
extern const CLzmaEncProps default_props;

/* ISeqInstream struct implementation */
struct seq_in_stream{
    ISeqInStream in_stream; // need this for implementation
    FILE * fd; // file to read from
};

/* ISeqInstream struct implementation */
struct seq_out_stream{
    ISeqOutStream out_stream; // need this for implementation
    FILE *fd; // file to write to
};

/* implementation of ISeqinstream, see ISeqInStream struct
 * for why we have these fields
 */
int in_stream_read(void *p, void *buf, size_t *size);

/* open a file and read raw data from it, then pass this data to
 * compress_data.
 * INPUT:
 * char *in_path - path to the file we want to open
 * char *out_path - path to file we will write compressed data to
 * void *args - available compression options see CLzmaEncprops in
 * LzmaEnc.h for more info
 * OUTPUT:
 * 1 - success
 * 0 - failure
 */
int compress_file(const char *in_path,
                  const char *out_path = NULL,
                  const CLzmaEncProps *args = &default_props);

/* decompress a compressed file, barely modified from lzmautil
 * thank @flowing water for rushing me
 * INPUT:
 * const char *in_path - string containing path to compressed file
 * const char *out_path - string  containing path to store
 * uncompressed file
 * void *args available parameters, see CLzmaDec in LzmaDec.h
 */
int decompress_file(const char *in_path,
                    const char *out_path = NULL);

/* compress data incrementally, it works using fn
 * callbacks that are implemented as static functions.
 * these functions just provide implementation for
 * Iseqinstream->read and Iseqoutstream->write
 * INPUT:
 * FILE *input - file stream to input file
 * FILE *output - file stream to ouput file
 * char *args - arguments to pass to the encode fn,
 * look at LzmaEnc.h for more info (props struct)
 * OUTPUT:
 * not implemented yet
 */
int compress_data_incr(FILE *input, FILE *output,
                       const CLzmaEncProps *args = &default_props);

/* decompress data incrementally, reads file size and then calculates
 * how much more is left to read based on how much data has alrdy been
 * processed, it knows its done when its processed "file_size" bytes of
 * data
 * INPUT:
 * FILE *input - filestream to input file (compressed filee)
 * FILE *output - filestream to desired destination
 * OUTPUT:
 * n/a at the moment
 */
int decompress_data_incr(FILE *input, FILE *output);
#endif // _LZMA2_WRAPPER_H
