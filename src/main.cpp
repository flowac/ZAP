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
	constexpr uint8_t TEST_KEYWORDS_LIM[] = {7, 7, 8, 6, 10, 7, 10};
	int32_t j, k;
	int32_t nPacks, nTrans, ndn;
	uint64_t i;
	bool val;
	uint8_t kt, xt[MAGNET_XT_LEN], tr[MAGNET_TR_LEN];
	uint8_t src[ED448_LEN], dest[ED448_LEN], sig[ED448_SIG_LEN];
	char dn[MAGNET_DN_LEN];
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
			memset(dn, 0, MAGNET_DN_LEN);
			k = rand() % 70 + 40;
			for (k = 0; k < MAGNET_XT_LEN; ++k) xt[k] = rand() % MAX_U8;
			for (ndn = 0, k = rand() % 11 + 1; k > 0; --k)
			{
				if (rand() % 3 == 0) dn[ndn++] = '0' + (rand() % 10);
				else
				{
					strncpy(dn + ndn, WORDS_EN.get(rand() % (WORDS_EN.size() + 1)).c_str(), MAGNET_DN_LEN - ndn - 1);
					ndn = strlen(dn);
				}
				if (ndn >= (MAGNET_DN_LEN - 1)) break;
				dn[ndn] = ' ';
				dn[++ndn] = 0;
			}

			kt = (rand() % (TEST_KEYWORDS_LIM[0] - 1)) + 1;
			kt |= (rand() % TEST_KEYWORDS_LIM[kt]) << 4;
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
	stemWord(buf, &len);
	if (strcmp(buf, expected) != 0)
	{
		printf("\tOriginal [%02lu]: %s\n", strlen(str), str);
		printf("\tStemmed  [%02u]: %s\n", len, buf);
		return false;
	}
	return true;
}

void line_proc_test(const char *str)
{
	char buf[BUF64];
	char *ut = NULL;
	int i, len = strlen(str);
	uint8_t kt[MAGNET_KT_LEN];
	uint32_t st[MAGNET_ST_LEN];

	if (len >= (int) BUF64)
	{
		printf("ERROR: input string exceeded buffer size %u\n", BUF64);
		return;
	}

	strcpy(buf, str);
	filterLine(buf, &len);
	printf("\tOriginal [%02lu]: %s\n", strlen(str), str);
	printf("\tFiltered [%02u]: %s\n", len, buf);

	strcpy(buf, str);
	encodeMsg(buf, st, ut, kt);
	printf("\tUT: %s<\n\tST: ", ut ? ut : "nil");
	for (i = 0; i < MAGNET_ST_LEN && st[i]; ++i) printf("%6u ", st[i]);
	printf("\n\tKT: ");
	for (i = 0; i < MAGNET_KT_LEN; ++i) printf("%2u ", kt[i]);
	printf("\n");

	if (ut) free(ut);
}

void print_search_terms(uint8_t kt1, uint8_t kt2, const char *str)
{
	if (isKeywordValid(kt1, kt2)) printf("[%s:%s]", getKeyword1(kt1), getKeyword2(kt1, kt2));
	else printf("Category invalid [%d:%d]", kt1, kt2);
	printf(" %s\n", str ? str : "nil");
}

void print_search_terms(const char *kt1, const char *kt2, const char *str)
{
	printf("[%s:%s] %s\n", kt1 ? kt1 : "nil", kt2 ? kt2 : "nil", str ? str : "nil");
}

template <class T_kt>
void search_test(torDB *td, T_kt kt1, T_kt kt2, const char *str = NULL, uint8_t verbose = 0)
{
	std::vector<uint32_t> result{searchTorDB(td, kt1, kt2, str, verbose)};
	print_search_terms(kt1, kt2, str);
	for (uint32_t idx : result) printf("%u ", idx);
	printf("\n");
}

void interact_test(chain *ch, torDB *td)
{
	bool tmpb;
	char buf[BUF64], *tmpc;
	uint8_t kt1, kt2, verbose;
	uint32_t blen, tmpu;

	if (!ch || !td) return;
	memset(buf, 0, BUF64);
PRINT_HELP:
	printf("Interactive test: {required} [optional] [v[v]]\n"
		   "lb           Get block length\n"
		   "a{i}b        Audit block at i\n"
		   "b{i}         Print block at i (v)\n"
		   "lm           Get magnet length\n"
		   "a{i}m        Audit magnet at i\n"
		   "m{i}         Print magnet at i (v)\n"
		   "s[i[i]] [n]  Search for magnet by category index i and name n (vv)\n"
		   "e            Exit\n");

	while (fgets(buf, BUF64, stdin))
	{
		for (blen = 0; buf[blen] && (buf[blen] == ' ' || isalnum(buf[blen])); ++blen);
		for (verbose = 0; blen && tolower(buf[blen - 1]) == 'v'; ++verbose, --blen);
		buf[blen] = 0;
		if (blen-- < 1) goto PRINT_HELP;
		tmpu = strtoul(buf + 1, NULL, 0);
		buf[blen] = tolower(buf[blen]);

		switch (tolower(buf[0]))
		{
		case 'l':
			if (buf[1] == 'b') printf("Block length is %lu\n", ch->blk.size());
			else if (buf[1] == 'm') printf("Magnet length is %lu\n", td->pak.size());
			else goto PRINT_HELP;
			break;
		case 'a':
			if (buf[blen] == 'b')
			{
				if (tmpu >= ch->blk.size()) goto PRINT_HELP;
				tmpb = checkBlock(&(ch->blk[tmpu]), false, tmpu ? ch->blk[tmpu - 1].crc : NULL);
				printf("Audit for block %u %s\n", tmpu, tmpb ? "passed" : "failed");
			}
			else if (buf[blen] == 'm')
			{
				if (tmpu >= td->pak.size()) goto PRINT_HELP;
				tmpb = checkPack(&(td->pak[tmpu]), false);
				printf("Audit for pack %u %s\n", tmpu, tmpb ? "passed" : "failed");
			}
			else goto PRINT_HELP;
			break;
		case 'b':
			if (tmpu >= ch->blk.size()) goto PRINT_HELP;
			blockToText(&(ch->blk[tmpu]), stdout, verbose);
			break;
		case 'm':
			if (tmpu >= td->pak.size()) goto PRINT_HELP;
			packToText(&(td->pak[tmpu]), stdout, verbose);
			break;
		case 's':
			tmpc = buf + 2;
			kt1 = isdigit(buf[1]) ? buf[1] - '0' : 0;
			kt2 = (kt1 && buf[2]) ? strtoul(buf + 2, &tmpc, 0) : 0;
			search_test(td, kt1, kt2, tmpc, verbose);
			break;
		case 'e':
			return;
			break;
		default:
			goto PRINT_HELP;
			break;
		}
	}
}

void chain_test(int size, bool interact)
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
	printTorWordMap(&td);
	search_test(&td, "Games", "PC");
	search_test(&td, "Games", "Mac");
	search_test(&td, "Games", "");
#endif

	if (interact) interact_test(&ch, &td);

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

void dictionary_search(void)
{
	char buf[BUF64];

	memset(buf, 0, BUF64);
	printf("Dictionary search test (enter any word, or 0 to exit):\n");

	while (fgets(buf, BUF64, stdin))
	{
		if (buf[0] == '0') break;
		for (uint8_t i = 0; i < BUF64; ++i) if (!isalpha(buf[i])) buf[i] = 0;

		std::vector<std::string> list = WORDS_EN.findN(buf, 9);
		for (std::string item : list) printf("%s ", item.c_str());
		printf("\n");
	}
}

int main(int argc, char **argv)
{
	bool interact = false; // Enter interactive mode once the test chain is loaded
	time_t tm;
	srand((unsigned) time(&tm));

	for (int i = 1; i < argc; ++i)
	{
		switch (tolower(argv[i][0]))
		{
		case 'd': dictionary_search(); return 0; break;
		case 'i': interact = true; break;
		default: printf("WARNING: unknown switch %s ignored.\n", argv[i]); break;
		}
	}

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

	printf("[INFO] Line processing test\n");
	line_proc_test("Inferno Beyond The 7th Circle v1 0 16-Razor1911");

	log_test();
	tracker_test();
	chain_test(100, interact);
	wallet_test();

	//std::cout.imbue(std::locale()); // might be useful to remove valgrind false positives
	log_deinit();
	return 0;
}
