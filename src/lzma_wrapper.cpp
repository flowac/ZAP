/**
 * @file lzma_wrapper.cpp
 * @brief Implementation of lzma Library wrapper
 */
#include <iostream>
#include <fstream>
#include <ios>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#ifdef _WIN32
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif//PATH_MAX
#else
#include <linux/limits.h>
#endif//WINDOWS

/* include */
#include "lzma_wrapper.h"
#include "log.h"
/* extern */
#include "C/LzmaLib.h"
#include "C/7zTypes.h"
#include "C/Alloc.h"
#include "C/LzmaEnc.h"
#include "C/LzmaDec.h"

/**
 * @brief Function implementation for struct ISzAlloc 
 * see examples in LzmaUtil.c and C/alloc.c
 *
 * @return Returns a ptr allocated space
 */
static void *SzAlloc(void *p, //!< Not implemented
                     size_t size //!< Amount to malloc
                     )
{
    (void)p; // silence unused var warning
    return MyAlloc(size); // just a malloc call...
}

/**
 * @brief Function implementation for struct ISzAlloc 
 * see examples in LzmaUtil.c and C/alloc.c
 */
static void SzFree(void *p, //!< Not implemented
                   void *address //!< Ptr to where we will call free
                   )
{
    (void)p; // silence unused var warning
    MyFree(address); // just a free call ... 
}

/** @brief Struct implementation */
ISzAlloc g_Alloc = {SzAlloc, SzFree};

/**
 * @brief Read raw data from a filedescriptor
 * INPUT:
 * @return
 * Size of data read\n
 * @p data Will contain the information read from fd
 */
static size_t my_read_data(FILE *fd, //!< Fd to input
                           void *data, //!< Buf to hold data
                           size_t data_len //!< Size to read, also indicates if the file is done being read
                           )
{
    if (data_len == 0)
        return 0;
    return fread(data, sizeof(unsigned char), data_len, fd);
}

/** @brief Implementation of ISeqoutstream->read
 *
 * @return
 * Currently only @c SZ_OK will change later, for valule see 7zTypes.h
 * @p data Will contain the information read from @p p->fd
 */
static int read_data(void *p, /**< Ptr to a seq_in_stream*/
                     void *data, /**< Buffer for data*/
                     size_t *data_len /**< Amount of data to be read*/
                     )
{
    if (*data_len == 0)
        return SZ_OK;
    *data_len = my_read_data(((seq_in_stream *)p)->fd, data, *data_len);
        return SZ_OK;
}

/** @brief Write raw data to a filedescriptor
 * 
 * @return
 * Number of bytes written
 */
static size_t my_write_data(FILE *fd, //!< Fp to dest
                            const void *data, //!< Data to write
                            size_t data_len //!< Size of @p data 
                            ) 
{
    if (data_len == 0)
        return 0;
    return fwrite(data, sizeof(unsigned char), data_len, fd);
}

/** @brief implementation of ISeqinstream->write
 *
 * @return
 * Number of bytes written
 */
static size_t write_data(void *p, //!< ptr to \link seq_out_stream \endlink
                         const void *data, //!< Data to write
                         size_t data_len //!< Size of @p data
                         )
{
    return my_write_data(((seq_out_stream *)p)->fd, data, data_len );
}

/** @brief Open two files, one for input one for output
 *
 * @return
 * 1 - success \n
 * 0 - failure
 */
static int open_io_files(const char *in_path, //!< Input path
                         const char*out_path, //!< Output path
                         FILE *fd[] //!< Array of FP, should change to individual Fps
                         )
{
    fd[0] = fopen(in_path, "rb");
    if (!fd[0]) {
        log_msg_default;
        goto cleanup;
    }
    fd[1] = fopen(out_path, "wb+");
    if (!fd[1])  {
        char msg[60];
        snprintf(msg, 59, "\nOpening [%s] failed\n", out_path);
        log_msg_custom(msg);
        goto cleanup;
    }
    return 1;

 cleanup:
    if (fd[0])
        fclose(fd[0]);
    if (fd[1])
        fclose(fd[1]);
    return 0;
}

/** @brief Get params of the 7z file
 *
 * Read the prop of the 7z file and then read the following 8 bits that
 * hold the file uncompressed size, these are stored in little endian
 *
 * @return
 * @p props_header Will contain the prop\n
 * Will return the size of the file\n
 * 0 - on failure (not implemented)
 */
static unsigned long get_header(FILE *fd, //!< Fp to 7z file
                                unsigned char *props_header, //!< Buf to hold the prop
                                size_t len //*< Size of the prop
                                )
{
    unsigned long rt = 0; // return val (file size)
    if (len != my_read_data(fd, props_header, len))
        return 0;
    for (int i = 0; i < 8; i++) {
        /* get file size, stored little endian after prop */
        rt |= (unsigned long)(props_header[LZMA_PROPS_SIZE + i]
                             << (i * 8));
    }
    return rt;
}

/** @brief update the props values with what was passed */
static void assign_prop_vals(CLzmaEncProps *prop_info, //!< Prop to update
                             const CLzmaEncProps *args //!< Prop with updated vals
                             )
{
    prop_info->level = args->level;
    prop_info->dictSize = args->dictSize;
    prop_info->reduceSize = args->reduceSize;
    prop_info->lc = args->lc;
    prop_info->lp = args->lp;
    prop_info->pb = args->pb;
    prop_info->algo = args->algo;
    prop_info->fb = args->fb;
    prop_info->btMode = args->btMode;
    prop_info->numHashBytes = args->numHashBytes;
    prop_info->mc = args->mc;
    prop_info->writeEndMark = args->writeEndMark;
    prop_info->numThreads = args->numThreads;
}

/** @brief Set the outputs file name for compression
 *
 * If out_path is null it will set the output name by concatting .7z to
 * input name, this is for compression only
 *
 * @return
 * @p out_path_local will hold the correct out_path
 */
static void set_comp_out_file_name(const char *in_path, //!< Input path
                                   const char *out_path, //!< Output path
                                   char *out_path_local //!< Output path after fn call
                                   )
{
    if (out_path == NULL)
        snprintf(out_path_local, PATH_MAX,
                 "%s.7z", in_path);
    else
        snprintf(out_path_local, PATH_MAX,
                 "%s", out_path);
}

/** @brief Set the output filename for decompression
 *
 * If out_path is null set the output file name by putting a string
 * terminator where .7z is
 *
 * @return
 * @p out_path_local will hold the correct output path
 */
static void set_decomp_out_file_name(const char *in_path, //!< Input path
                                     const char *out_path, //!< Output path
                                     char *out_path_local //!< Output path after fn call
                                     )
{
    if (out_path == NULL) {
        snprintf(out_path_local, PATH_MAX,
                 "%s", in_path);
        char * tmp = strstr(out_path_local,".7z");
        tmp[0] = '\0';
    }
    else {
        snprintf(out_path_local, PATH_MAX,
                 "%s", out_path);
    }
}

/* work in progress, wrapper fn for compression call */
int compress_file(const char *in_path,
                  const char *out_path,
                  const CLzmaEncProps *args)
{
    if (in_path == NULL) {
        log_msg("Invalid args to compress_file");
        return 0;
    }
    char out_path_local[PATH_MAX];
    set_comp_out_file_name(in_path, out_path,
                           out_path_local);

    FILE *fd[2]; /* i/o file descriptors */
    fd[0] = 0; fd[1] = 0;

    char msg[200];
    snprintf(msg, 199, "  compressing %s -> %s\n", in_path, out_path_local);
    std::cout << msg;
    
    /* open i/o files, return fail if this failes */
    if (!open_io_files(in_path, out_path_local, fd))
        return 0;
    compress_data_incr(fd[0], fd[1], args);

    if (fd[0] != NULL)
        fclose(fd[0]);
    if (fd[1] != NULL)
        fclose(fd[1]);
    return 1;
}

int decompress_file(const char *in_path,
                    const char *out_path)
{
    if (in_path == NULL) {
        log_msg("Invalid args to decompress_file");
        return 0;
    }
    char out_path_local[PATH_MAX];
    set_decomp_out_file_name(in_path, out_path,
                             out_path_local);

    FILE *fd[2]; /* i/o file descriptors */
    fd[0] = 0; fd[1] = 0;
    
    /* open i/o files, return fail if this failes */
    if (!open_io_files(in_path, out_path_local, fd))
        return 0;
    char msg[200];
    snprintf(msg, 199, "  decompressing %s -> %s\n", in_path, out_path_local);
    std::cout << msg;
    if (!decompress_data_incr(fd[0], fd[1]))
        log_msg("Failed to decompress\n");

    if (fd[0])
        fclose(fd[0]);
    if (fd[1])
        fclose(fd[1]);
    return 1;
}

int compress_data_incr(FILE *input, FILE *output, const CLzmaEncProps *args)
{
    int rt = 1;
    /* iseqinstream and iseqoutstream objects */
    seq_in_stream i_stream = {{read_data}, input};
    seq_out_stream o_stream = {{write_data}, output};
    /* CLzmaEncHandle is just a pointer (void *) */
    CLzmaEncHandle enc_hand = LzmaEnc_Create(&g_Alloc);
    if (enc_hand == NULL) {
        log_msg_custom("Error allocating mem when"
                       "reading in stream");
        return SZ_ERROR_MEM;
    }
    /* 5 bytes for lzma prop + 8 bytes for filesize */
    unsigned char props_header[LZMA_PROPS_SIZE_FILESIZE];
    SizeT props_size = LZMA_PROPS_SIZE; // size of prop
    unsigned long file_size = get_file_size_c(input); // filesize
    CLzmaEncProps prop_info; // info for prop, control vals for comp
    /* note the prop is the header of the compressed file */
    LzmaEncProps_Init(&prop_info);
    assign_prop_vals(&prop_info, args);
    rt = LzmaEnc_SetProps(enc_hand, &prop_info);
    if (rt != SZ_OK)
        goto end;
    
    rt = LzmaEnc_WriteProperties(enc_hand, props_header, &props_size);
    for (int i = 0; i < 8; i++) {
        /* store filesize little endian after prop, easier
        * to read back in
        */
        props_header[props_size++] = (unsigned char)(file_size
                                                     >> (8 * i));
    }
    write_data((void*)&o_stream, props_header, props_size);
    if (rt == SZ_OK)
        rt = LzmaEnc_Encode(enc_hand,
                            &(o_stream.out_stream),
                            &(i_stream.in_stream),
                            NULL, &g_Alloc, &g_Alloc);
    LzmaEnc_Destroy(enc_hand, &g_Alloc, &g_Alloc);
    return 1;

 end:
    log_msg_custom_errno("Error occurred compressing data: LZMA errno", rt);
    LzmaEnc_Destroy(enc_hand, &g_Alloc, &g_Alloc);
    return rt;
}

int decompress_data_incr(FILE *input, FILE *output)
{
    unsigned long file_size = 0; // size of file
    int rt; // return val
    unsigned char props_header[LZMA_PROPS_SIZE_FILESIZE];
    CLzmaDec state; // view in LzmaDec.h

    file_size = get_header(input, props_header, LZMA_PROPS_SIZE_FILESIZE);
    if (file_size == 0) {
        log_msg("Failed to get file size");
        return 0; // failed
    }
    LzmaDec_Construct(&state);
    rt = LzmaDec_Allocate(&state, props_header, LZMA_PROPS_SIZE, &g_Alloc);
    if (rt != SZ_OK) {
        log_msg_custom_errno("Failed call to LzmaDec_Allocate", rt);
        return 0;
    }
    unsigned char in_buff[buffer_cread_size];
    unsigned char out_buff[buffer_cread_size];
    unsigned long out_pos = 0, in_read_size = 0, in_pos = 0;
    SizeT in_processed = 0, out_processed = 0;
    ELzmaFinishMode fin_mode = LZMA_FINISH_ANY;
    ELzmaStatus status;
    LzmaDec_Init(&state);
    while(1) {
        if (in_pos == in_read_size) {
            in_read_size = my_read_data(input, in_buff, buffer_cread_size);
            in_pos = 0;
        } else {
            in_processed = (SizeT)(in_read_size - in_pos);
            out_processed = (SizeT)(buffer_cread_size - out_pos);
            if (out_processed > file_size) {
                out_processed = (unsigned int)file_size;
                fin_mode = LZMA_FINISH_END;
            }
            rt = LzmaDec_DecodeToBuf(&state,
                                     out_buff + out_pos,
                                     &out_processed,
                                     in_buff + in_pos,
                                     &in_processed,
                                     fin_mode,
                                     &status);
            in_pos += in_processed;
            out_pos += out_processed;
            file_size -= out_processed;

            my_write_data(output, out_buff, out_pos);
            out_pos = 0;

            if ((rt != SZ_OK) || (file_size == 0))
                break;
            if ((in_processed == 0) && (out_processed == 0)) {
                log_msg_custom("ERROR OCCURRED DECOMPRESS\n");
                break;
            }
        }
    }
    LzmaDec_Free(&state, &g_Alloc);
    return 1;
    
}
