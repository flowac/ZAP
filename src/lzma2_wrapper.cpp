#include <iostream>
#include <fstream>
#include <ios>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
/* include */
#include "lzma2_wrapper.h"
#include "log.h"
/* extern */
#include "C/LzmaLib.h"
#include "C/7zTypes.h"
#include "C/Alloc.h"
#include "C/LzmaEnc.h"

/* function implementation for struct ISzAlloc 
 * see examples in LzmaUtil.c and C/alloc.c
 */

static void *SzAlloc(void *p, size_t size)
{
    (void)p; // silence unused var warning
    return MyAlloc(size); // just a malloc call...
}

/* function implementation for struct ISzAlloc 
 * see examples in LzmaUtil.c and C/alloc.c
 */
static void SzFree(void *p, void *address)
{
    (void)p; // silence unused var warning
    MyFree(address); // just a free call ... 
}

ISzAlloc g_Alloc = {SzAlloc, SzFree};

/* Read raw data from a filedescriptor
 * INPUT:
 * FILE *fd - file descriptor
 * unsigned char *data - buffer that will hold the read
 * data
 * size_t *data_len - the size of the read data, this will
 * also indicate when we have reached the end of the file
 * OUTPUT:
 * size of data read
 */
static size_t my_read_data(FILE *fd, void *data, size_t data_len)
{
    if (data_len == 0)
        return 0;
    return fread(data, sizeof(unsigned char), data_len, fd);
}

/* implementation of ISeqoutstream->read */
static int read_data(void *p, void *data, size_t *data_len)
{
    if (*data_len == 0)
        return SZ_OK;
    *data_len = my_read_data(((seq_in_stream *)p)->fd, data, *data_len);
        return SZ_OK;
}

/* write raw data to a filedescriptor
 * INPUT:
 * FILE *fd - file descriptor
 * unsigned char *data - buffer holds the data
 * size_t *data_len - the size of the data buffer
 * OUTPUT:
 * number of bytes written
 */
static size_t my_write_data(FILE *fd, const void *data, size_t data_len)
{
    if (data_len == 0)
        return 0;
    return fwrite(data, sizeof(unsigned char), data_len, fd);
}

/* implementation of ISeqinstream->write */
static size_t write_data(void *p, const void *data, size_t data_len)
{
    return my_write_data(((seq_out_stream *)p)->fd, data, data_len );
}

/* Open two files, one for input one for output
 * INPUT:
 * const char *in_path - path to input file
 * const char *out_path - path to output file
 */
static int open_io_files(const char *in_path, const char*out_path,
                         FILE *fd[])
{
    fd[0] = fopen(in_path, "r");
    if (fd[0] == NULL)  {
        log_msg("Error opening input file for compression: %s\n",
                strerror(errno));
        goto cleanup;
    }
    fd[1] = fopen(out_path, "w+");
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

/* work in progress, wrapper fn for compression call */
int compress_file(char *in_path, char *out_path)
{
    FILE *fd[2]; /* i/o file descriptors */
    //unsigned char *input; /* i/o buffers */
    //input = (unsigned char*)malloc(sizeof(unsigned char)*25696714);
    //if (input == NULL)
    //    printf("lol\n");
    //unsigned char output[buffer_cread_size];
    //int count = 1;
    //size_t input_len = 25696714; /* buffer sizes */
    //size_t output_len = buffer_cread_size;

    /* open i/o files, return fail if this failes */
    if (!open_io_files(in_path, out_path, fd))
        return 0;
    /* read the file */
    //while(read_data((void *)fd[0], input, &input_len)) {
    //    /* this is not accurate at the end,
    //     * a good approximate though */
    //    printf("%d bytes compressesd\n",
    //           count * buffer_cread_size);
    //    count++;
    //    /* compress data and write to output */
    //    compress_data(input, input_len,
    //                  output, &output_len);
    //    write_data((void *)fd[1], output, output_len);
    //}
    compress_data_incr(fd[0], fd[1]);


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

int compress_data_incr(FILE *input, FILE *output, char *args)
{
    int rt = 1;
    seq_in_stream i_stream = {{read_data}, input};
    seq_out_stream o_stream = {{write_data}, output};
    /* CLzmaEncHandle is just a pointer */
    CLzmaEncHandle enc_hand = LzmaEnc_Create(&g_Alloc);
    if (enc_hand == NULL) {
        log_msg ("Error allocating mem when reading stream");
        return SZ_ERROR_MEM;
    }
    /* 5 bytes for lzma prop + 8 bytes for filesize */
    unsigned char props_header[LZMA_PROPS_SIZE + 8];
    unsigned int props_size = LZMA_PROPS_SIZE;
    long file_size = get_file_size_c(input);
    CLzmaEncProps prop_info;
    /* create the prop, note the prop is the header of the
     * compressed file
     */
    LzmaEncProps_Init(&prop_info);
    /* if prop wasnt initialized correctly return fail */
    rt = LzmaEnc_SetProps(enc_hand, &prop_info);
    if (rt != SZ_OK)
        goto end;

    rt = LzmaEnc_WriteProperties(enc_hand, props_header, &props_size);
    /* write filesize to prop header and then to file */
    for (int i = 0; i < 8; i++)
        props_header[props_size++] = (unsigned char)(file_size >> (8 * i));
    write_data((void*)&o_stream, props_header, props_size);
    /* do the compression */
    if (rt == SZ_OK)
        rt = LzmaEnc_Encode(enc_hand,
                            &(o_stream.out_stream),
                            &(i_stream.in_stream),
                            NULL, &g_Alloc, &g_Alloc);
    LzmaEnc_Destroy(enc_hand, &g_Alloc, &g_Alloc);
    return 1;

 end:
    log_msg("Error occurred compressing data: LZMA errno %d\n", SZ_OK);
    LzmaEnc_Destroy(enc_hand, &g_Alloc, &g_Alloc);
    return rt;
}

int decompress_data_incr(FILE *input, FILE *output)
{
    
}
