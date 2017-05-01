#include "alib.h"

int main()
{
    /*time_t raw;
    time(&raw);
    printf(" >time: %ld\n",raw);*/

    chain *ch = newChain();

    for (uint32_t i = 0; i < 2048; i++)
    {
        uint64_t key = 0xFFFF0000FFFF0000 + i;
        uint32_t len = 4;
        uint64_t *payload = (uint64_t *)malloc(sizeof(uint64_t) * len);
        payload[0]=0x1FFF;
        payload[1]=0x2FFF;
        payload[2]=0x3FFF;
        payload[3]=0x4FFF;

        insertBlock(newBlock(key, len, payload), ch);
    }

    printBlock(ch->head[0]);
    printBlock(ch->head[2047]);

    deleteChain(ch);
    free(ch);

    std::cout.imbue(std::locale());
    return 0;
}
