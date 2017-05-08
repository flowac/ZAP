#include "alib.h"
#include "atype.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

int main()
{
    /*time_t raw;
    time(&raw);
    printf(" >time: %ld\n",raw);*/

    chain *ch = newChain();
    uint32_t size = 1000;

    for (uint32_t i = 0; i < size; i++)
    {
        uint64_t key = 0xFFFF0000FFFF0000 + i;
        uint32_t len = 4;
        pack **packs = (pack **)malloc(sizeof(pack *) * len);
        if (packs == NULL) break;
        for (uint32_t j = 0; j < len; j++)
        {
            packs[j] = newPack("dn:testtesttesttesttesttesttesttest",
                               4*1024*1024,
                               "xt:testtesttesttesttesttesttesttest",
                               "tr:testtesttesttesttesttesttesttest");
        }
        
        if (!insertBlock(newBlock(key, len, packs), ch)) break;
    }
    printBlock(ch->head[0]);
    printBlock(ch->head[size - 1]);
    
    printf("Free'd %lu bytes\n", deleteChain(ch) + sizeof(chain));

    free(ch);

    std::cout.imbue(std::locale());
    return 0;
}
