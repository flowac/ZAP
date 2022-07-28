/**
 * @file lzma_wrapper.h
 * @brief Lzma library implementation
 *
 * Still have to switch over to make use lzmamt - future goals
 */
#ifndef _LZMA2_WRAPPER_H
#define _LZMA2_WRAPPER_H

#include <string>
#include "7zTypes.h"
#include "LzmaEnc.h"

/** @brief Default buffer size for i/o (64KB) */
#define buffer_cread_size (1<<16)

/** @brief Size of prop and the following data that contains the filesize */
#define LZMA_PROPS_SIZE_FILESIZE LZMA_PROPS_SIZE + 8

/**
 * @brief Default prop values
 *
 * To customize vals you can edit default prop struct or do:\n
 * CLzmaEncProps myprop = default_prop;\n
 * Then customize the structs vals, see lzmalib.h for
 * more info.\n
 * Then pass your struct as the third arg to compressfile
 */
const CLzmaEncProps default_props = {
	1,            //!< level
	1 << 16,      //!< dictSize
	3,            //!< lc
	0,            //!< lp
	2,            //!< pb
	0,            //!< algo
	32,           //!< fb
	0,            //!< btMode
	4,            //!< numHashbytes
	32UL,         //!< mc
	0,            //!< writeEndmark
	2,            //!< numThreads
	0x7FFFFFFFULL,//!< reduceSize
	0             //!< affinity
};

const CLzmaEncProps slow_props = {
	1,            //!< level
	1 << 24,      //!< dictSize
	3,            //!< lc
	0,            //!< lp
	2,            //!< pb
	1,            //!< algo
	32,           //!< fb
	0,            //!< btMode
	4,            //!< numHashbytes
	32UL,         //!< mc
	0,            //!< writeEndmark
	2,            //!< numThreads
	0x7FFFFFFFULL,//!< reduceSize
	0             //!< affinity
};

/**
 * @brief ISeqInstream struct implementation 
 */
struct seq_in_stream {
	ISeqInStream in_stream;// need this for implementation
	FILE *fd;              // file to read from
};

/**
 * @brief ISeqOutstream struct implementation
 */
struct seq_out_stream {
	ISeqOutStream out_stream;// need this for implementation
	FILE *fd;                // file to write to
};

/**
 * @brief Implementation of ISeqinstream, see ISeqInStream struct
 * for why we have these fields
 */
int in_stream_read(void *p, void *buf, size_t *size);

/**
 * @brief Compress a file from a pathname
 *
 * Open a file and read raw data from it, then pass this data to
 * compress_data.
 * @return
 * 1 - success\n
 * 0 - failure
 */
bool compress_file(const char *in_path,        //!< Path to the input file
				   const char *out_path = NULL,//!< Path to the output
				   const CLzmaEncProps *args = &default_props);//!< Arguments for compression see CLzmaEncProps in LzmaEnc.h for more info

/**
 * @brief Decompress a compressed file, barely modified from lzmautil
 *
 * This implementation should be redone
 */
bool decompress_file(const char *in_path,         //!< Path to compressed file
					 const char *out_path = NULL);//!< Path to destination, default will just chop off .7z

/**
 * @brief Compress data incrementally
 *
 * It works using fn callbacks that are implemented as
 * static functions. these functions just provide implementation for
 * Iseqinstream->read and Iseqoutstream->write
 * 
 * @return
 * Not implemented yet
 */
int compress_data_incr(FILE *input, //!< Fp to input file
                       FILE *output,//!< Fp to output file
                       const CLzmaEncProps *args = &default_props);//!< Arguments to pass to the encode fn

/**
 * @brief Decompress data incrementally
 *
 * Reads file size and then calculates how much more is left to read
 * based on how much data has alrdy been processed, it knows its
 * done when its processed "file_size" bytes of data
 * 
 * @return
 * Not implemented yet
 */
int decompress_data_incr(FILE *input,  //!< Fp to compressed file
                         FILE *output);//!< Fp to dest
#endif //_LZMA2_WRAPPER_H
