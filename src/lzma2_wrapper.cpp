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

/* Read raw data from a filedescriptor
 * INPUT:
 * FILE *fd - file descriptor
 * unsigned char *data - buffer that will hold the read
 * data
 * size_t *data_len - the size of the read data, this will
 * also indicate when we have reached the end of the file
 * OUTPUT:
 * 1 - more data
 * 0 - end of file
 * -1 failure
 */
static int read_data(FILE *fd, unsigned char *data, size_t *data_len)
{
    *data_len = fread(data, sizeof(unsigned char),
                      buffer_cread_size, fd);

    if (*data_len == buffer_cread_size)
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

int compress_file(const char *in_path, const char *out_path)
{
    FILE *fd[2]; // input and output fd
    unsigned char input[buffer_cread_size]; // i/o buffers
    unsigned char output[buffer_cread_size];
    int rt = 1; // return value
    int count = 1;
    size_t input_len = buffer_cread_size; // buffer sizes
    size_t output_len = buffer_cread_size;

    fd[0] = fopen(in_path, "r");
    if (fd[0] == NULL)  {
        log_msg("Error opening input file for compression: %s\n",
                strerror(errno));
        rt = 0;
        goto cleanup;
    }
    fd[1] = fopen(out_path, "w");
    if (fd[0] == NULL)  {
        log_msg("Error opening input file for compression: %s\n",
                strerror(errno));
        rt = 0;
        goto cleanup;
    }

    read_data(fd[0], input, &input_len);
    do {
        /* this is not accurate at the end,
         * a good approximate though */
        printf("%d bytes compressesd\n",
               count * buffer_cread_size);
        count++;
        compress_data(input, input_len,
                      output, &output_len);
        write_data(fd[1], output, output_len);
    } while(read_data(fd[0], input, &input_len));


 cleanup:
    if (fd[0] != NULL)
        fclose(fd[0]);
    if (fd[1] != NULL)
        fclose(fd[1]);
    return rt;
}

int compress_data(unsigned char *input, size_t input_len,
                  unsigned char *output, size_t *output_len)
{
    size_t props_size = LZMA_PROPS_SIZE;
    //*output_len = 2 * input_len / 3 + 128;
    /* &output[LZMA_PROPS_SIZE] specifies where to store the data
     * &output_len stores the size of the output data
     * input specifies the input data
     * input_len specifies the size of the input data
     * &output[0] specifies where to store prop info
     * &props_size specifies the size of the prop info
     */
    LzmaCompress(&output[LZMA_PROPS_SIZE], output_len,
                 input, input_len,
                 &output[0], &props_size,
                 -1, // level
                 0,  // dictSize
                 -1, // lc (literal context bits)
                 -1, // lp (literal position bits)
                 -1, // pb (position bits)
                 -1, // fb (word size)
                 -1); // numThreads (number of threads)
    return 1;
}
