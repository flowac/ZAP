#include "atype.h"
#include "alib.h"
#include "alibio.h"
#include "ssl_fn.h"
#include "log.h"
#include "lzma_wrapper.h"

#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define N_THREADS 5
#define N_TEST_BLOCKS 1000

/* Test if ssl_fn.c create_sha1sum is working correctly
 *
 */
void sha1_test()
{
    unsigned char *tmp = NULL;
    tmp = create_sha1sum((char *)"shatest.file");
    if(tmp != NULL) {
        for (int i = 0; i < 20; i++) {
            printf("%02x",tmp[i]);
        }
        printf("\n");
        free(tmp);
    }
}

/* Test if log.c log_msg is working correctly
 *
 */
void log_test()
{
    log_msg("fewf %s\n", "wfean");
}

void zip_test()
{
    compress_file("t2","t2.my7z", NULL);
}

chain *chain_gen(uint64_t size)
{
    uint64_t i;
    uint16_t j, k, nPack;
    const char charset[] = "qazwsxedcrfvtgbyhnujmikolpQAZWSXEDCRFVTGBYHNUJMIKOLP0123456789";//62
    
    uint64_t key;
    chain *ch = newChain();
    char *dn = (char *)malloc(sizeof(char) * 121);
    
    for (i = 0; i < size && i < MAX_U32; i++) {
        nPack  = rand() % 50 + 50;
        pack **packs = (pack **)malloc(sizeof(pack *) * nPack);
        
        for (j = 0; j < nPack; j++) {
            k = rand() % 90 + 30;
            dn[k] = 0;
            for (k--; k > 0; k--) {
                dn[k] = charset[rand()%62];
            }
            dn[0] = '0';
            
            packs[j] = newPack(dn, (rand()%50+1)*1024*1024, dn, dn);
        }
        
        key = rand() % MAX_U16 * MAX_U32;
        if (!insertBlock(newBlock((uint32_t)i, key, nPack, packs), ch))
            break;
    }
    free(dn);
    return ch;
}

typedef struct
{
    char in7z[64];
    char outf[64];
}decompParams;

void *decompress_wrap(void *args)
{
    decompParams *dp = (decompParams *)args;
    decompress_file(dp->in7z, dp->outf);
    return NULL;
}

void uncompress_test()
{
    pthread_t threads[N_THREADS];
    decompParams dp[N_THREADS];

    for (int i = 0; i < N_THREADS; i++) {
        sprintf(dp[i].in7z,"temp%d.file.7z", i+1);
        sprintf(dp[i].outf, "temp%d.1unc",i+1);
        pthread_create(&threads[i], NULL, &decompress_wrap, (void *)&dp[i]);
    }
    for (int i = 0; i < N_THREADS; i++) pthread_join(threads[i], NULL);
}

void chain_test()
{
    printf("\nGenerating\n");
    chain *ch = chain_gen(N_TEST_BLOCKS);
    
    printf("Compressing\n");
    uint32_t tmp;
#ifndef WINDOWS
    struct timespec tmp1,tmp2;
    clock_gettime(CLOCK_MONOTONIC, &tmp1);//Start
    chainCompactor(ch, N_THREADS);
    clock_gettime(CLOCK_MONOTONIC, &tmp2);//End
    tmp = (tmp2.tv_sec - tmp1.tv_sec) * 1000 + (tmp2.tv_nsec - tmp1.tv_nsec) / 1000000;
#else
    tmp = GetTickCount();
    chainCompactor(ch, N_THREADS);
    tmp = GetTickCount() - tmp;
#endif
    printf("Took %d milliseconds\n", tmp);
    
    printf("\nFree'd %lu bytes\n", deleteChain(ch) + sizeof(chain));
    free(ch);
    
    uncompress_test();
}

void decompress_test()
{
    printf("\nGenerating\n");
    chain *ch = chain_gen(N_TEST_BLOCKS);

    uint32_t tmp;
#ifndef WINDOWS
    struct timespec tmp1,tmp2;
    clock_gettime(CLOCK_MONOTONIC, &tmp1);//Start
#else
    tmp = GetTickCount();
#endif

    chainToText_to_file(ch, 1);
    chainCompactor(ch, N_THREADS);
    printf("\nFree'd %lu bytes\n", deleteChain(ch) + sizeof(chain));
    free(ch);
    ch = chain_extractor("temp%s.file",5);
    chainToText_to_file(ch, 2);
    //FILE * fp = fopen("orig1.file", "r");
    //chain *tmp_chain = newChain();
    //text2Chainz(fp, tmp_chain);
    //chainToText_to_file(tmp_chain, 2);
    printf("\nFree'd %lu bytes\n", deleteChain(ch) + sizeof(chain));
    free(ch);
    //fclose(fp);

#ifndef WINDOWS
    clock_gettime(CLOCK_MONOTONIC, &tmp2);//End
    tmp = (tmp2.tv_sec - tmp1.tv_sec) * 1000 + (tmp2.tv_nsec - tmp1.tv_nsec) / 1000000;
#else
    tmp = GetTickCount() - tmp;
#endif
    printf("Took %d milliseconds\n", tmp);
    
}

int main()
{
//    log_test();
//    chain_test();//Depreciated
//    zip_test();
    chain_test();
//    decompress_test();
//    sha1_test();

    //std::cout.imbue(std::locale());//Might be usefull to remove valgrind false positives
    return 0;
}

