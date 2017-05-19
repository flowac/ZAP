#include <iostream>
#include <fstream>
#include <ios>
/* include */
#include "lzma2_wrapper.h"
/* extern */
#include "Lzma2Encoder.h"

int compress_lzma2(const char *in_path, const char *out_path)
{
    /* encoder object */
    /* create and open streams */
    NCompress::NLzma2::CEncoder *to_compress = new NCompress::NLzma2::CEncoder;
    std::ofstream in_stream;
    std::ofstream out_stream;
    in_stream.open(in_path, std::ios_base::in);
    out_stream.open(out_path, std::ios_base::out);

    std::cout << "opened" << std::endl;

    in_stream.close();
    out_stream.close();

    std::cout << "closed" << std::endl;
    return 1;
}
