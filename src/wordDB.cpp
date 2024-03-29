#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

wordDB WORDS_EN("extern/scrap/english.src");
wordDB STOPWORDS_EN("extern/scrap/stopwords_en.src");

wordDB::wordDB(const char *src)
{
	char buf[BUF64];
	FILE *fp = fopen(src, "r");
	uint32_t blen, tlen;

	if (!fp) return;
	if (!fgets(buf, BUF64, fp)) return;
	if ((tlen = strtoul(buf, NULL, 0)) >= MAX_U32) return;
	if (!(dict = (char **) calloc(tlen, sizeof(char *)))) return;

	while (fgets(buf, BUF64, fp) && len < tlen)
	{
		blen = strlen(buf);
		while (blen && !isalpha(buf[blen - 1])) --blen;
		if (!blen) continue;

		dict[len] = (char *) calloc(blen + 1, 1);
		memcpy(dict[len++], buf, blen);
	}
	printf("[INFO] Dictionary %s initialized %u of out %u expected\n", src, len, tlen);
	fclose(fp);
}

wordDB::~wordDB()
{
	while (len) free(dict[--len]);
	free(dict);
}

uint32_t wordDB::find(const char *str)
{
	int cmp = -1;
	uint32_t i, left = 0, right = len - 1;
	if (len == 0) return 0UL;

	while (left < right)
	{
		i = (left + right) >> 1;
		cmp = strcasecmp(str, dict[i]);
		if (cmp == 0) return ++i;
		if (cmp < 0) right = i;
		else          left = i + 1;
	}

	return 0UL;
}

std::vector<std::string> wordDB::findN(const char *str, uint8_t n)
{
	int cmp = -1, slen = strlen(str);
	uint32_t i = 0, left = 0, right = len - 1;
	std::vector<std::string> ret;
	if (len == 0 || slen == 0) return ret;

	while (left < right)
	{
		i = (left + right) >> 1;
		cmp = strcasecmp(str, dict[i]);
		if (cmp == 0) break;
		if (cmp < 0) right = i;
		else          left = i + 1;
	}
	for (; i < len && strncasecmp(str, dict[i], slen) > 0; ++i);

    for (uint8_t j = 0; j < n && i < len; ++i, ++j)
	{
		if (strncasecmp(str, dict[i], slen) != 0) break;
		ret.push_back(std::string(dict[i]));
	}

	return ret;
}

std::string wordDB::get(uint32_t idx)
{
	if (idx < 1 || idx >= len) return std::string();
	return std::string(dict[--idx]);
}

uint32_t wordDB::size(void)
{
	return len;
}
