/* YO @Flowing-water we gotta start plopping GPL all up in here */
   
#include "ssl_fn.h"
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *create_sha1sum(char *dst)
{
    SHA_CTX ctx; // sha1 struct (look at sha.h)
    unsigned char *sha1sum = NULL; // sha1sum dest
    char buffer[file_input_buff_size]; // buffer for file i/o
    FILE *p_dst = NULL; // fd to dst
    
    p_dst = fopen(dst, "r");
    if (!p_dst) {
	/* we gonna have a log fn? */
	perror("fopen");
	goto end;
    }

    // wtf spartan naming and camel case? nice libssl
    if (!SHA1_Init(&ctx))// init the struct
	goto end; // ssl calls return 0 on fail

    // read file in
    while (fgets(buffer, file_input_buff_size, p_dst)) {
	// update teh sha1sum with what we read
	if(!SHA1_Update(&ctx,
			(void*)buffer,
			strlen(buffer))) // u like this syntax @ flowing? u mad bro?
	    goto end;
    }
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
