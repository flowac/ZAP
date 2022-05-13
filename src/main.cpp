#include "atype.h"
#include "alib.h"
#include "alib_io.h"
#include "alib_wallet.h"
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

void chain_gen(chain *ch, uint64_t size)
{
	int32_t j, k;
	int32_t nPacks;
	int32_t nTrans;
	uint64_t i;
	const char charset[] = "qazwsxedcrfvtgbyhnujmikolpQAZWSXEDCRFVTGBYHNUJMIKOLP0123456789";//62

	bool val;
	uint8_t xt[MAGNET_XT_LEN];
	char dn[121];
	uint8_t tr[121];
	char *kt[MAGNET_KT_COUNT];

	for (i = 0; i < size; i++)
	{
		nPacks = rand() % 200 + 50;
		nTrans = 0;

		for (j = 0; j < nPacks; j++)
		{
			memset(xt, 0, MAGNET_XT_LEN);
			memset(dn, 0, 121);
			memset(tr, 0, 121);
			k = rand() % 70 + 40;
			for (; k >= 0; --k)
			{
				if (k < MAGNET_XT_LEN) xt[k] = rand() % MAX_U8;
				dn[k] = charset[rand() % 62];
				tr[k] = charset[rand() % 62];
			}
			uint8_t nkt = rand() % MAGNET_KT_COUNT;
			for (k = 0; k < MAGNET_KT_COUNT; kt[k++] = NULL);
			for (k = 0; k < nkt; ++k)
			{
				uint8_t len = 1 + rand() % (MAGNET_KT_LEN - 1);
				kt[k] = (char *) calloc(len + 1, 1);
				for (uint8_t x = 0; x < len; ++x) kt[k][x] = charset[rand() % 62];
			}

			pack packs;
			tran trans;
			val = newPack(&packs, xt, (rand() % 50 + 1) * 1024 * 1024, dn, tr, kt);
			if (!val)
			{
				for (int x = 0; x < MAGNET_KT_COUNT; ++x) if (kt[x]) free(kt[x]);
				//printf("    newPack failed?");
			}
			else if (!enqueuePack(&packs)) printf("    enqueuePack failed?");
		}

		if (!newBlock(ch)) printf("    newBlock failed at %lu?", i);
	}
}

void chain_test(int size)
{
	const char *zaaFile = "temp.zaa"; // chainToZip file
	const char *za2File = "temp.za2"; // imported, then chainToZip'd file
	const char *txtFile = "temp.txt"; // chainToText output
	chain ch, cin1, cin2;

	start_timer();

	printf("\nGenerate\n");
	start_timer();
	chain_gen(&ch, size);
	print_elapsed_time();

	printf("\nWrite to file\n");
	start_timer();
	chainToText(&ch, txtFile);
	print_elapsed_time();

	printf("\nWrite to zip\n");
	start_timer();
	chainToZip(&ch, zaaFile);
	print_elapsed_time();

	printf("\nAudit: ");
	if (auditChain(&ch))
		printf("OK\n");
	else
		printf("FAIL\n");

	checksum_test(zaaFile);

	printf("\nImport 1\n");
	start_timer();
	if (!chainFromZip(&cin1, zaaFile)) printf("> failed!\n");
	print_elapsed_time();

	printf("\nWrite to zip 2\n");
	start_timer();
	chainToZip(&cin1, za2File);
	print_elapsed_time();

	printf("\nAudit: ");
	if (auditChain(&cin1))
		printf("OK\n");
	else
		printf("FAIL\n");

	uint64_t ccomp = compareChain(&ch, &cin1);
	printf("Chain compare status %lu (%s)\n", ccomp, ccomp == MAX_U64 ? "OK" : "BAD");
	deleteChain(&ch);
	deleteChain(&cin1);
	checksum_test(za2File);

#if 0
	printf("\n7zip zip\n");
	start_timer();
	compress_file(zaaFile, "temp.zaa.zip");
	print_elapsed_time();

	start_timer();
	decompress_file("temp.zaa.zip", "temp.zaa.unz");
	print_elapsed_time();

	start_timer();
	compress_file(zaaFile, "temp.zaa.zip2", &slow_props);
	print_elapsed_time();

	start_timer();
	decompress_file("temp.zaa.zip2", "temp.zaa.unz2");
	print_elapsed_time();

	start_timer();
	checksum_test(zaaFile);
	checksum_test("temp.zaa.unz");
	print_elapsed_time();
#endif

	print_elapsed_time();
}

void wallet_test()
{
	uint64_t deci = 77;
	uint16_t frac = 44;
	uint8_t sig[ED448_SIG_LEN];
	uint8_t pub[ED448_LEN];
	uint8_t priv[ED448_LEN];
	uint8_t dest[ED448_LEN];
	memset(sig, 0, ED448_SIG_LEN);
	memset(pub, 0, ED448_LEN);
	memset(priv, 0, ED448_LEN);
	memset(dest, 0, ED448_LEN);

	const uint64_t msgLen = 10;
	uint8_t msg[msgLen];
	u64Packer(msg, deci);
	msg[8] = frac | MAX_U8;
	msg[9] = frac >> 8;

	if (!newWallet(pub, priv)) printf("failed wallet\n");
	else printf("wallet OK\n");
	if (!sendToAddress(dest, priv, sig, deci, frac)) printf("failed send\n");
	else printf("send OK\n");
	if (!verifyMessage(pub, sig, msg, msgLen)) printf("failed verify\n");
	else printf("verify OK\n");
}

int main()
{
	time_t tm;
	srand((unsigned) time(&tm));

	printf("OpenSSL version %lX.%lX.%lX (required was 3.0.0+)\n",
		   (OPENSSL_VERSION_NUMBER >> 28) & MAX_U4,
		   (OPENSSL_VERSION_NUMBER >> 20) & MAX_U8,
		   (OPENSSL_VERSION_NUMBER >>  4) & MAX_U8);
	log_test();
	if (!importPack("extern/scrap/pirate.txt")) printf("Pack import failed\n");
	chain_test(200);
	wallet_test();

	//std::cout.imbue(std::locale()); // might be useful to remove valgrind false positives
	log_deinit();
	return 0;
}
