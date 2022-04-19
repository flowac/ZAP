#include "atype.h"
#include "alib.h"
#include "alibio.h"
#include "ssl_fn.h"
#include "time_fn.h"
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

chain *chain_gen(uint64_t size)
{
	uint16_t j, k;
	uint32_t nPacks;
	uint32_t nTrans;
	uint64_t i;
	const char charset[] = "qazwsxedcrfvtgbyhnujmikolpQAZWSXEDCRFVTGBYHNUJMIKOLP0123456789";//62

	bool val;
	block bx;
	chain *ch = newChain();
	char dn[121], xt[121], tr[121];

	for (i = 0; i < size; i++) {
		nPacks = rand() % 50 + 50;
		pack *packs = (pack *) calloc(nPacks, sizeof(pack));
		nTrans = 0;
		tran *trans = NULL;

		for (j = 0; j < nPacks; j++) {
			k = rand() % 90 + 30;
			dn[k] = xt[k] = tr[k] = 0;
			for (--k; k > 0; --k) {
				dn[k] = charset[rand() % 62];
				xt[k] = charset[rand() % 62];
				tr[k] = charset[rand() % 62];
			}
			dn[0] = xt[0] = tr[0] = '0';

			val = newPack(&packs[j], dn, (rand() % 50 + 1) * 1024 * 1024, xt, tr);
			if (!val) printf("    newPack failed?");
		}

		val = newBlock(&bx, i, 0, nPacks, packs, nTrans, trans);
		if (!val) printf("    newBlock failed?");

		if (!insertBlock(&bx, ch)) break;
	}
	return ch;
}

void chain_test(int size)
{
	const char *zaaFile = "temp.zaa"; // chainToZip file
	const char *zipFile = "temp.zip"; // 7zip compress file
	const char *txtFile = "temp.txt"; // chainToText output
	const char *unzFile = "temp.unz"; // unzipped file

	start_timer();

	printf("\nGenerate\n");
	start_timer();
	chain *ch = chain_gen(size);
	print_elapsed_time();

	printf("\nWrite to file\n");
	start_timer();
	chainToText(ch, txtFile);
	print_elapsed_time();

	printf("\nWrite to zip\n");
	start_timer();
	chainToZip(ch, zaaFile);
	print_elapsed_time();

	deleteChain(ch);
	free(ch);

	#if false
	printf("\n7zip text\n");
	start_timer();
	compress_file(txtFile, zipFile);
	decompress_file(zipFile, unzFile);
	print_elapsed_time();

	printf("\n7zip zip\n");
	start_timer();
	compress_file(zaaFile, "temp.zaa.zip");
	decompress_file("temp.zaa.zip", "temp.zaa.unz");
	print_elapsed_time();

	start_timer();
	checksum_test(txtFile);
	checksum_test(unzFile);
	checksum_test(zipFile);
	checksum_test(zaaFile);
	checksum_test("temp.zaa.unz");
	print_elapsed_time();

	#endif
	print_elapsed_time();
}

int main()
{
	time_t tm;
	srand((unsigned) time(&tm));

	log_test();
	chain_test(2000);

	//std::cout.imbue(std::locale()); // might be useful to remove valgrind false positives
	log_deinit();
	return 0;
}
