#include <iostream>
#include <fstream>
#include <ios>
/* include */
#include "lzma2_wrapper.h"
#include "log.h"
/* extern */
#include "Lzma2Encoder.h"

/*
CMyComPtr<ISequentialInStream> setup_in_stream(char *in_path)
{
    CMyComPtr<ISequentialInStream> file_stream;
    CInFileStream *in_stream_spec = new CInFileStream;
    file_stream = in_stream_spec;
    const UString &input_name = in_path;
    // open stream
    if (!in_stream_spec->Open(us2fs(input_name))) {
        log_msg("Failed to open input file for compression: %s",
                in_path);
    }
    return file_stream; 
    } */

int compress_lzma2(const char *in_path, const char *out_path)
{
    NCompress::NLzma2::CEncoder to_compress;
    //CMyComPtr<ISequentialInStream> in_stream = setup_in_stream(in_path); // input stream
    std::ifstream in_stream;
    std::ofstream out_stream;
    in_stream.open(in_path);
    out_stream.open(out_path);
    UInt64 tmp = 8161825;

    std::cout << "opened" << std::endl;
    to_compress.Code((ISequentialInStream*)&in_stream,
                     (ISequentialOutStream*)&out_stream,
                     NULL,
                     &tmp,
                     NULL);

    std::cout << "closed" << std::endl;
    return 1;
}
