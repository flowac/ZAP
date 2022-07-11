#include "main_lib.h"
#include "file_io.h"
#include "wallet.h"
#include "ssl_fn.h"
#include "time_fn.h"
#include "log.h"
#include "lzma_wrapper.h"

#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *test_tracker = "udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.openbittorrent.com%3A6969%2Fannounce&tr=udp%3A%2F%2F9.rarbg.to%3A2710%2Fannounce&tr=udp%3A%2F%2F9.rarbg.me%3A2780%2Fannounce&tr=udp%3A%2F%2F9.rarbg.to%3A2730%2Fannounce&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=http%3A%2F%2Fp4p.arenabg.com%3A1337%2Fannounce&tr=udp%3A%2F%2Ftracker.torrent.eu.org%3A451%2Fannounce&tr=udp%3A%2F%2Ftracker.tiny-vps.com%3A6969%2Fannounce&tr=udp%3A%2F%2Fopen.stealth.si%3A80%2Fannounce";

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
	uint8_t *sum = check_sha3_from_file(src, &len);
	if (!pstat(sum != NULL, "Checksum test")) return;

	printf("[INFO] %-14s [%02d]: ", src, len);
	for (i = 0; i < len; i++)
	{
		printf("%02x", sum[i]);
	}
	printf("\n");

	if (sum) free(sum);
}

void chain_gen(chain *ch, torDB *td, uint64_t size)
{
	int32_t j, k;
	int32_t nPacks;
	int32_t nTrans;
	uint64_t i;
	const char charset[] = "qazwsxedcrfvtgbyhnujmikolpQAZWSXEDCRFVTGBYHNUJMIKOLP0123456789";//62

	bool val;
	uint8_t kt, xt[MAGNET_XT_LEN], tr[MAGNET_TR_LEN];
	uint8_t src[ED448_LEN], dest[ED448_LEN], sig[ED448_SIG_LEN];
	char dn[121];
	memcpy(tr, test_tracker, strlen(test_tracker) + 1);
	memset(src,  0xC0, ED448_LEN);
	memset(dest, 0xFF, ED448_LEN);
	memset(sig,  0xEE, ED448_SIG_LEN);

	for (i = 0; i < size; i++)
	{
		nPacks = rand() % 200 + 50;
		nTrans = 20;

		for (j = 0; j < nPacks; j++)
		{
			memset(xt, 0, MAGNET_XT_LEN);
			memset(dn, 0, 121);
			k = rand() % 70 + 40;
			for (; k >= 0; --k)
			{
				if (k < MAGNET_XT_LEN) xt[k] = rand() % MAX_U8;
				dn[k] = charset[rand() % 62];
			}

			kt = 1;
			val = newPack(td, xt, (rand() % 50 + 1) * 1024 * 1024, dn, tr, kt);
			if (!val) printf("    newPack failed?");
		}

		for (j = 0; j < nTrans; j++)
		{
			val = newTran(NULL, 1337, 42, 8008, src, dest, sig);
		}

		if (!newBlock(ch)) printf("    newBlock failed at %lu?", i);
	}
}

bool word_proc_test(const char *str, const char *expected)
{
	char buf[BUF64];
	int len = strlen(str);
	if (len >= (int) BUF64)
	{
		printf("ERROR: input string exceeded buffer size %u\n", BUF64);
		return false;
	}

	strcpy(buf, str);
	stem_word(buf, &len);
	if (strcmp(buf, expected) != 0)
	{
		printf("\tOriginal [%02lu]: %s\n", strlen(str), str);
		printf("\tStemmed  [%02u]: %s\n", len, buf);
		return false;
	}
	return true;
}

bool line_proc_test(const char *str)
{
	char buf[BUF64];
	char *ut = NULL;
	int i, len = strlen(str);
	uint8_t kt[8];
	uint64_t *st = NULL;

	if (len >= (int) BUF64)
	{
		printf("ERROR: input string exceeded buffer size %u\n", BUF64);
		return false;
	}

	strcpy(buf, str);
	filter_line(buf, &len);
	printf("\tOriginal [%02lu]: %s\n", strlen(str), str);
	printf("\tFiltered [%02u]: %s\n", len, buf);

	strcpy(buf, str);
	encode_msg(buf, &st, &ut, kt);
	printf("\tUT: %s<\n\tST: ", ut ? ut : "nil");
	for (i = 0; st; ++i)
	{
		printf("%6lu %6lu %6lu ", st[i] & MAX_U21, (st[i] >> 21) & MAX_U21, (st[i] >> 42) & MAX_U21);
		if (st[i] >> 63) break;
	}
	printf("\n\tKT: ");
	for (i = 1; i < 8; ++i) printf("%2u ", kt[i]);
	printf("\n");

	if (ut) free(ut);
	if (st) free(st);
	return true;
}

void search_test(torDB *td, const char *kt1, const char *kt2, const char *str)
{
	std::vector<uint32_t> result{searchTorDB(td, kt1, kt2, str)};
	printf("[%s %s] %s:\n\t", kt1, kt2, str);
	for (uint32_t idx : result) printf("%u ", idx);
	printf("\n");
}

void chain_test(int size)
{
	const char *zaaFile = "temp.zaa"; // chainToZip file
	const char *za2File = "temp.za2"; // imported, then chainToZip'd file
	const char *blkText = "temp.txt"; // chainToText output
	const char *torFile = "temp.tor"; // trackers
	const char *torText = "temp.tor.txt";
	chain ch, cin1;
	torDB td, tin1;

	start_timer();

	pstat(torDBFromTxt(&td, "extern/scrap/pirate.src"), "TorDB import from text");

	printf("\nGenerate\n");
	start_timer();
	chain_gen(&ch, &td, size);
	print_elapsed_time();
#if 0
	printTorCat(&td);
	search_test(&td, "Games", "PC", NULL);
	search_test(&td, "Games", "Mac", NULL);
	search_test(&td, "Games", NULL, NULL);
#endif

	printf("\nWrite to file\n");
	start_timer();
	chainToText(&ch, blkText);
	torDBToText(&td, torText);
	print_elapsed_time();

	printf("\nWrite to zip 1\n");
	start_timer();
	chainToZip(&ch, zaaFile);
	torDBToZip(&td, torFile);
	print_elapsed_time();

	pstat(auditChain(&ch), "Chain audit");
	pstat(auditTorDB(&td), "TorDB audit");
	checksum_test(zaaFile);
	checksum_test(torFile);

	start_timer();
	pstat(chainFromZip(&cin1, zaaFile), "Chain import");
	pstat(torDBFromZip(&tin1, torFile), "TorDB import");
	print_elapsed_time();

	printf("\nWrite to zip 2\n");
	start_timer();
	chainToZip(&cin1, za2File);
	torDBToZip(&tin1, torFile);
	print_elapsed_time();

	pstat(auditChain(&cin1), "Chain audit");
	pstat(auditTorDB(&tin1), "TorDB audit");

	uint64_t ccomp = compareChain(&ch, &cin1);
	if (!pstat(ccomp == MAX_U64, "Chain compare"))
		printf("[INFO] Difference at %lu\n", ccomp);
	deleteChain(&ch);
	deleteChain(&cin1);
	checksum_test(za2File);
	checksum_test(torFile);

#if 0
	printf("\n7zip benchmark\n");
	start_timer();
	compress_file(zaaFile, "temp.zaa.7z");
	print_elapsed_time();

	start_timer();
	decompress_file("temp.zaa.7z", "temp.zaa.unz");
	print_elapsed_time();

	start_timer();
	compress_file(zaaFile, "temp.zaa.slow.7z", &slow_props);
	print_elapsed_time();

	start_timer();
	decompress_file("temp.zaa.slow.7z", "temp.zaa.slow.unz");
	print_elapsed_time();

	start_timer();
	checksum_test(zaaFile);
	checksum_test("temp.zaa.unz");
	print_elapsed_time();
#endif

	print_elapsed_time();
}

void tracker_test()
{
	char tr3[MAGNET_TR_LEN];
	uint8_t tr2[MAGNET_TR_LEN];
	uint32_t len, clen, dlen;

	if ((len = strlen(test_tracker)) >= MAGNET_TR_LEN)
	{
		printf("[ERROR] Tracker size limit reached [%u/%u]. Test aborted.\n", len, MAGNET_TR_LEN);
		return;
	}
	memcpy(tr2, test_tracker, len + 1);

	clen = compressTracker(tr2);
	dlen = decompressTracker(tr2, tr3);
	if (!pstat(len == dlen && memcmp(test_tracker, tr3, len + 1) == 0, "Tracker operations"))
		printf("[INFO] Original: %s\n[INFO] Decomped: %s\n", test_tracker, tr3);
	printf("[INFO] Tracker size: %u,  compressed: %u,  decompressed: %u\n", len, clen, dlen);
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

	pstat(newWallet(pub, priv), "Wallet");
	pstat(sendToAddress(dest, priv, sig, deci, frac), "Send");
	pstat(verifyMessage(pub, sig, msg, msgLen), "Verify");
}

int main()
{
	time_t tm;
	srand((unsigned) time(&tm));

	printf("[INFO] OpenSSL version %lX.%lX.%lX (required was 3.0.0+)\n",
		   (OPENSSL_VERSION_NUMBER >> 28) & MAX_U4,
		   (OPENSSL_VERSION_NUMBER >> 20) & MAX_U8,
		   (OPENSSL_VERSION_NUMBER >>  4) & MAX_U8);

	pstat(word_proc_test("aliens", "alien") &&
		  word_proc_test("hoping", "hope") &&
		  word_proc_test("hopping", "hop") &&
		  word_proc_test("transfer", "transfer") &&
		  word_proc_test("performer", "perform") &&
		  word_proc_test("refreshments", "refresh") &&
		  word_proc_test("sterilizations", "steril"),
		  "Word processing test");

	// TODO: define line proc failure conditions or expected output
	pstat(line_proc_test("I have a dream-1984") &&
		  line_proc_test("Beavis & Butthead S1e2"),
		  "Line processing test");

	log_test();
	tracker_test();
	chain_test(100);
	wallet_test();

	//std::cout.imbue(std::locale()); // might be useful to remove valgrind false positives
	log_deinit();
	return 0;
}
