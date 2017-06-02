/* YO @Flowing-water we gotta start plopping GPL all up in here */
   
#include "ssl_fn.h"
#include "log.h"
#include "time_fn.h"

#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

unsigned char *create_sha1sum(const char *dst)
{
    SHA_CTX ctx; // sha1 struct (look at sha.h)
    unsigned char *sha1sum = NULL, // sha1sum dest
	buffer[file_input_buff_size]; // buffer for file i/o
    FILE *p_dst = NULL; // fd to dst
    size_t read_size = 0;
    
    p_dst = fopen(dst, "r");
    if (!p_dst) {
        /* we gonna have a log fn? */
        log_msg("%s %s File: %s Function: %s \n", get_loc_time(), strerror(errno),__FILE__,__PRETTY_FUNCTION__);
        goto end;
    }

    // wtf spartan naming and camel case? nice libssl
    if (!SHA1_Init(&ctx))// init the struct
        goto end; // ssl calls return 0 on fail

    // read file in
    do {
        read_size = fread(buffer, sizeof(unsigned char), file_input_buff_size, p_dst);
        // update teh sha1sum with what we read
        if(!SHA1_Update(&ctx, (void*)buffer, read_size))
            goto end;
    } while (read_size == file_input_buff_size); // fread returns less than that size on failure

    // create hash
    sha1sum = (unsigned char*)malloc(sizeof(char) * SHA_DIGEST_LENGTH);
    if (!sha1sum)
	goto end;
    SHA1_Final(sha1sum, &ctx);

 end:
    if(p_dst)
	fclose(p_dst);
    return sha1sum;
}
