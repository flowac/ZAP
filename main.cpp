#include "alib.h"

int main()
{
    /*time_t raw;
    time(&raw);
    printf(" >time: %ld\n",raw);*/

    chain *ch = newChain();
    uint32_t size = 10000;

    for (uint32_t i = 0; i < size; i++)
    {
        uint64_t key = 0xFFFF0000FFFF0000 + i;
        uint32_t len = 4;
        uint64_t *payload = (uint64_t *)malloc(sizeof(uint64_t) * len);
        if (payload == NULL) break;
//        printf("%p\t", payload);
        payload[0]=0x1FFF;
        payload[1]=0x2FFF;
        payload[2]=0x3FFF;
        payload[3]=0x4FFF;

        if (!insertBlock(newBlock(key, len, payload), ch)) break;
    }
    printBlock(ch->head[0]);
    printBlock(ch->head[size - 1]);
//getchar();
    printf("Free'd %lu bytes\n", deleteChain(ch) + sizeof(chain));

    free(ch);

    std::cout.imbue(std::locale());
    return 0;
}
