#include "alib.h"
#include "atype.h"
#include "ssl_fn.h"
#include "log.h"

#include "../extern/7z/LzmaAlone.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Test if ssl_fn.c create_sha1sum is working correctly
 * 
 */
void sha1_test()
{
    unsigned char *tmp = NULL;
    tmp = create_sha1sum("/home/gator/Downloads/Torrent/tmp.txt");
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
    log_msg("i love %s\n", "allan");
}

void chain_test()
{
    chain *ch = newChain();
    uint32_t size = 1000;

    for (uint32_t i = 0; i < size; i++) {
        uint64_t key = 0xFFFF0000FFFF0000 + i;
        uint32_t len = 4;
        pack **packs = (pack **)malloc(sizeof(pack *) * len);
        if (packs == NULL)
            break;
        for (uint32_t j = 0; j < len; j++) {
            packs[j] = newPack("dn:testtesttesttesttesttesttesttest",
                               4*1024*1024,
                               "xt:testtesttesttesttesttesttesttest",
                               "tr:testtesttesttesttesttesttesttest");
        }
        
        if (!insertBlock(newBlock(key, len, packs), ch))
            break;
    }
    printBlock(ch->head[0]);
    printBlock(ch->head[size - 1]);
    
    printf("Free'd %lu bytes\n", deleteChain(ch) + sizeof(chain));

    free(ch);

    std::cout.imbue(std::locale());
}

int main()
{
    sha1_test();
    log_test();
    chain_test();

    return 0;
}
