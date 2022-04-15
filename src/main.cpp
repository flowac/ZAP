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

/* Test if log.c log_msg is working correctly
 *
 */
void log_test()
{
	log_msg("fewf %s\n", "wfean");
}

void zip_test()
{
	compress_file("t2", "t2.my7z", NULL);
}

chain *chain_gen(uint64_t size)
{
//#define rand() (33)
	uint16_t j, k;
	uint64_t nPack;
	uint64_t i;
	const char charset[] = "qazwsxedcrfvtgbyhnujmikolpQAZWSXEDCRFVTGBYHNUJMIKOLP0123456789";//62

	uint64_t key;
	block bx;
	chain *ch = newChain();
	char dn[121];

	for (i = 0; i < size; i++) {
		nPack = rand() % 50 + 50;
		pack *packs = (pack *) malloc(sizeof(pack) * nPack);

		for (j = 0; j < nPack; j++) {
			k = rand() % 90 + 30;
			dn[k--] = 0;
			for (; k > 0; --k) {
				dn[k] = charset[rand() % 62];
			}
			dn[0] = '0';

			bool val = newPack(&packs[j], dn, (rand() % 50 + 1) * 1024 * 1024, dn, dn);
		}

		key = rand() % MAX_U16 * MAX_U32;
		newBlock(&bx, 0, i, key, &nPack, &packs);
		if (!insertBlock(&bx, ch)) break;
	}
	return ch;
}

void chain_test()
{
	printf("\nGenerating\n");
	chain *ch = chain_gen(B_MAX);

	printf("Compressing\n");
	uint32_t tmp;
#ifdef _WIN32
	tmp = GetTickCount();
	chainCompactor(ch);
	tmp = GetTickCount() - tmp;
#else
	struct timespec tmp1, tmp2;
	clock_gettime(CLOCK_MONOTONIC, &tmp1); // start
	chainCompactor(ch);
	clock_gettime(CLOCK_MONOTONIC, &tmp2); // end
	tmp = (tmp2.tv_sec - tmp1.tv_sec) * 1000
		+ (tmp2.tv_nsec - tmp1.tv_nsec) / 1000000;
#endif
	printf("Took %d milliseconds\n", tmp);

	deleteChain(ch);
	printf("\nMemory free'd");
	free(ch);

	compress_file("temp.file", "temp.file.7z");
	decompress_file("temp.file.7z", "temp.1unc");
}

int main()
{
	log_test();
//zip_test();
	chain_test();

	//std::cout.imbue(std::locale()); // might be useful to remove valgrind false positives
	log_deinit();
	return 0;
}

