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
#include <time.h>

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
	printf("\nGenerate\n");
	chain *ch = chain_gen(2000);

	printf("Write to file ");
	uint32_t tmp;
	struct timespec tm1, tm2;
	timespec_get(&tm1, TIME_UTC); // start
	chainCompactor(ch);
	timespec_get(&tm2, TIME_UTC); // end
	tmp = (tm2.tv_sec - tm1.tv_sec) * 1000
		+ (tm2.tv_nsec - tm1.tv_nsec) / 1000000;
	printf("took %d milliseconds\n", tmp);

	deleteChain(ch);
	printf("\nMemory free'd");
	free(ch);

	compress_file("temp.file", "temp.file.7z");
	decompress_file("temp.file.7z", "temp.1unc");
}

void checksum_test(const char *src)
{
	uint32_t i, len;
	uint8_t *sum = check_sha3_512_from_file(src, &len);

	printf("%-14s [%02d]: ", src, len);
	for (i = 0; i < len; i++)
	{
		printf("%02x", sum[i]);
	}
	printf("\n");

	if (sum) free(sum);
}

int main()
{
	time_t tm;
	srand((unsigned) time(&tm));

	log_test();
	chain_test();

	checksum_test("temp.file");
	checksum_test("temp.1unc");
	checksum_test("temp.file.7z");

	//std::cout.imbue(std::locale()); // might be useful to remove valgrind false positives
	log_deinit();
	return 0;
}

