#include <iostream>
#include <fstream>
#include <ios>
#include <string.h>
#include <stdio.h>
#include <errno.h>
/* include */
#include "lzma2_wrapper.h"
#include "log.h"
/* extern */
#include "Lzma2Encoder.h"
#include "C/LzmaLib.h"
#include "C/7zTypes.h"
#include "C/Alloc.h"

/* function implementation for struct ISzAlloc 
 * see examples in LzmaUtil.c and C/alloc.c
 */
static void *SzAlloc(void *p, size_t size)
{
    p = p;
    return MyAlloc(size); // just a malloc call...
}
/* function implementation for struct ISzAlloc 
 * see examples in LzmaUtil.c and C/alloc.c
 */
static void SzFree(void *p, void *address)
{
    p = p;
    MyFree(address); // just a free call ... 
}
const ISzAlloc g_Alloc = {szAlloc, szFree};

/* Read raw data from a filedescriptor
 * INPUT:
 * FILE *fd - file descriptor
 * unsigned char *data - buffer that will hold the read
 * data
 * size_t *data_len - the size of the read data, this will
 * also indicate when we have reached the end of the file
 * OUTPUT:
 * 2 - more data
 * 1 - last chunk of data
 * 0 - end of file
 * -1 failure
 */
static int read_data(FILE *fd, unsigned char *data, size_t *data_len)
{
    *data_len = fread(data, sizeof(unsigned char),
                      buffer_cread_size, fd);

    if (*data_len == buffer_cread_size)
        return 2;
    else if (*data_len > 0)
        return 1;
    else 
        return 0;
}

/* write raw data to a filedescriptor
 * INPUT:
 * FILE *fd - file descriptor
 * unsigned char *data - buffer holds the data
 * size_t *data_len - the size of the data buffer
 * OUTPUT:
 * 1 - success
 * 0 - failure
 */
static int write_data(FILE *fd, unsigned char *data, size_t data_len)
{
    data_len = fwrite(data, sizeof(unsigned char),
                      data_len, fd);

    if (data_len == buffer_cread_size)
        return 1;
    else
        return 0;
}

static int open_io_files(const char *in_path, const char*out_path,
                         FILE *fd[])
{
    fd[0] = fopen(in_path, "r");
    if (fd[0] == NULL)  {
        log_msg("Error opening input file for compression: %s\n",
                strerror(errno));
        goto cleanup;
    }
    fd[1] = fopen(out_path, "w");
    if (fd[0] == NULL)  {
        log_msg("Error opening output file for compression: %s\n",
                strerror(errno));
        goto cleanup;
    }
    return 1;

 cleanup:
    if (fd[0] != NULL)
        fclose(fd[0]);
    if (fd[1] != NULL)
        fclose(fd[1]);
    return 0;
}

int compress_file(const char *in_path, const char *out_path)
{
    FILE *fd[2]; /* i/o file descriptors */
    unsigned char input[buffer_cread_size]; /* i/o buffers */
    unsigned char output[buffer_cread_size];
    int count = 1;
    size_t input_len = buffer_cread_size; /* buffer sizes */
    size_t output_len = buffer_cread_size;

    /* open i/o files, return fail if this failes */
    if (!open_io_files(in_path, out_path, fd))
        return 0;

    /* read the file */
    while(read_data(fd[0], input, &input_len)) {
        /* this is not accurate at the end,
         * a good approximate though */
        printf("%d bytes compressesd\n",
               count * buffer_cread_size);
        count++;
        /* compress data and write to output */
        compress_data(input, input_len,
                      output, &output_len);
        write_data(fd[1], output, output_len);
    }


    if (fd[0] != NULL)
        fclose(fd[0]);
    if (fd[1] != NULL)
        fclose(fd[1]);
    return 1;
}

int compress_data(unsigned char *input, size_t input_len,
                  unsigned char *output, size_t *output_len)
{
    size_t props_size = LZMA_PROPS_SIZE;
    /* &output[LZMA_PROPS_SIZE] specifies where to store the data
     * &output_len stores the size of the output data
     * input specifies the input data
     * input_len specifies the size of the input data
     * &output[0] specifies where to store prop info
     * &props_size specifies the size of the prop info
     */
    int rt = LzmaCompress(&output[LZMA_PROPS_SIZE], output_len,
                 input, input_len,
                 &output[0], &props_size,
                 -1, // level
                 0,  // dictSize
                 -1, // lc (literal context bits)
                 -1, // lp (literal position bits)
                 -1, // pb (position bits)
                 -1, // fb (word size)
                 -1); // numThreads (number of threads)
    if (rt != SZ_OK) {
        log_msg ("Failed to compress data: error %d", rt);
        return 0;
    }
    return 1;
}

int decompress_data(unsigned char *input, size_t input_len,
                    unsigned char *output, size_t *output_len)
{
    /* output - specifies where to store uncompressed data
     * output_len - stores the length of the uncompressed data
     * input[LZMA_PROPS_SIZE] - position of the data in the input buffer
     * &input_len - length of the input buffer
     * &input[0] - position of the prop info
     * LZMA_PROPS_SIZE - prop size
     */
    int rt = LzmaUncompress(output, output_len,
                            &input[LZMA_PROPS_SIZE], &input_len,
                            &input[0], LZMA_PROPS_SIZE);
    
    if (rt != SZ_OK) {
        log_msg ("Failed to decompress data: error %d", rt);
        return 0;
    }
    return 1;
}

int in_stream_read(void *p, void *buf, size_t *size)
{
    /* CLzmaEncHandle is just a pointer */
    CLzmaEncHandle enc_hand = LzmaEnc_Create(g_Alloc);
    if (enc_hand == NULL) {
        log_msg ("Error allocating mem when reading stream");
        return SZ_ERROR_MEM;
    }
    /* create the prop, note the prop is the header of the
     * compressed file
     */
    CLzmaEncProps prop_info;
    int rt = LzmaEncProps_Init(&prop_info);

    /* if prop wasnt initialized correctly return fail */
    if (rt != SZ_OK)
        goto end;


 end:
    LzmaEnc_Destroy(enc, &g_Alloc, g_Alloc);
    return rt;
}
