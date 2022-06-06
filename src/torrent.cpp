#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "main_lib.h"

typedef enum
{
	TZERO  = 1 << 7,
	TSTART = 0,
	TUDP   = 1,
	THTTP  = 2,
	TANN   = 3,
	TTOR   = 4,
	TTRA   = 5,
	TCOM   = 6,
	TORG   = 7,
	TOPEN  = 8,
	TMAX   = 9
} TR_ENUM;

const char *tlist[TMAX] =
{
	"&tr=",
	"udp",
	"http",
	"announce",
	"torrent",
	"tracker",
	".com",
	".org",
	"open"
};

uint32_t compressTracker(uint8_t *tr)
{
	char buf3[3] = {0, 0, 0};
	uint32_t ret = 0, len = 0, i, j, mark;
	if (!(len = u8len(tr))) return 0;

	for (i = 0; i < len; ++i, ++ret)
	{
		if (ret >= MAGNET_TR_LEN) [[unlikely]] return 0;
		tr[ret] = tr[i];
		if (tr[i] == '%')
		{
			tr[ret] = TZERO;
			for (mark = ret; i < len && tr[i] == '%'; i += 3)
			{
				if (i + 2 >= len) return 0;
				memcpy(buf3, tr + i + 1, 2);

				tr[mark]++;
				if (tr[mark] > MAX_U8 - TMAX) return 0;
				errno = 0;
				tr[++ret] = (uint8_t) strtol(buf3, NULL, 16);
				if (errno != 0) return 0;
			}
			--i;
		}
		else
		{
			for (j = 0; j < TMAX; ++j)
			{
				if ((mark = u8cmp(tr + i, (char *) (tlist[j]))))
				{
					tr[ret] = MAX_U8 - j;
					i += mark - 1;
					break;
				}
			}
		}
	}
	tr[ret] = 0;

	return ret;
}

uint32_t decompressTracker(uint8_t *tr, char ret[MAGNET_TR_LEN])
{
	int i, j;
	uint32_t retLen = 0;

	for (i = 0; tr[i]; ++i)
	{
		if (tr[i] < TZERO)
		{
			if (retLen + 1 >= MAGNET_TR_LEN) return 0;
			ret[retLen++] = tr[i];
		}
		else if (tr[i] <= MAX_U8 - TMAX)
		{
			for (j = tr[i++] - TZERO; j > 0; ++i, --j)
			{
				if (retLen + 3 >= MAGNET_TR_LEN) return 0;
				retLen += sprintf(ret + retLen, "%%%02X", tr[i]);
			}
			--i;
		}
		else
		{
			if (retLen + strlen(tlist[MAX_U8 - tr[i]]) >= MAGNET_TR_LEN) return 0;
			retLen += sprintf(ret + retLen, tlist[MAX_U8 - tr[i]]);
		}
	}
	ret[retLen] = 0;
	return retLen;
}

void printTorCat(torDB *target)
{
	std::map<std::string, category>::iterator itc;
	std::map<std::string, std::vector<uint32_t>>::iterator itu;

	printf("Categories %lu\n", target->cat.size());
	for (itc = target->cat.begin(); itc != target->cat.end(); ++itc)
	{
		printf("%s: ", itc->first.c_str());
		for (uint32_t u32 : itc->second.idx) printf("%u ", u32);
		printf("\nSub categories %lu ", itc->second.sub.size());
		for (itu = itc->second.sub.begin(); itu != itc->second.sub.end(); ++itu)
		{
			printf("%s, ", itu->first.c_str());
			for (uint32_t u32 : itu->second) printf("%u ", u32);
		}
		printf("\n");
	}
}

bool processPack(torDB *td, pack *px)
{
	category tCat;
	uint32_t idx;
	std::map<std::string, category>::iterator itc;
	std::map<std::string, std::vector<uint32_t>>::iterator itu;
	std::string key1, key2;
	if (!td || !px) return false;

	idx = td->pak.size() - 1;
	if (px->kt[0].empty()) return true;
	key1 = px->kt[0];
	if (px->kt[1].empty()) key2.clear();
	else key2 = px->kt[1];

	if ((itc = td->cat.find(key1)) == td->cat.end()) return false;
	else if (key2.empty()) td->cat[key1].idx.push_back(idx);
	else
	{
		tCat = td->cat[key1];
		if ((itu = tCat.sub.find(key2)) == tCat.sub.end()) return false;
		else td->cat[key1].sub[key2].push_back(idx);
	}

	return true;
}

bool newPack(torDB *td, uint8_t xt[MAGNET_XT_LEN], uint64_t xl, char *dn, uint8_t *tr, std::string kt[MAGNET_KT_COUNT])
{
	uint32_t ndn = 0, ntr = 0, i = 0;
	pack px = {.crc = {0}, .xt = {0}, .xl = 0ULL, .dn = NULL, .tr = NULL};
	if (!td || !xt || !dn || !tr) goto cleanup;

	if (!(ndn = strlen(dn)) || ndn > MAGNET_DN_LEN) goto cleanup;
	if (!(ntr = u8len(tr)) || ntr > MAGNET_TR_LEN) goto cleanup;
	if (!(ntr = compressTracker(tr))) goto cleanup;

	memcpy(px.xt, xt, MAGNET_XT_LEN);
	px.xl = xl;
	if (!(px.dn = (char *) calloc(ndn + 1, 1))) goto cleanup;
	memcpy(px.dn, dn, ndn);
	if (!(px.tr = (uint8_t *) calloc(ntr + 1, 1))) goto cleanup;
	memcpy(px.tr, tr, ntr);

	for (i = 0; i < MAGNET_KT_COUNT; ++i) px.kt[i].clear();
	for (i = 0; i < MAGNET_KT_COUNT; ++i)
	{
		if (0 == kt[i].size()) break;
		if (kt[i].size() > MAGNET_KT_LEN) goto cleanup;
		px.kt[i] = kt[i];
	}

	if (!checkPack(&px, true)) goto cleanup;
	td->pak.push_back(px);
	if (!processPack(td, &px)) printf("proc pack failed\n");

	return true;
cleanup:
	printf("\nfailed new pack kt[i] [%u] %s< ndn %u ntr %u %p\n", i, dn, ndn, ntr, tr);
	if (px.dn) free(px.dn);
	if (px.tr) free(px.tr);
	return false;
}

std::map<uint32_t, typename std::list<std::string>> searchTorDB(torDB *td, char *kt[MAGNET_KT_COUNT], const char *str)
{
	std::map<uint32_t, typename std::list<std::string>> result;

	return result;
}

inline void deletePack(pack *target)
{
	if (!target) return;
	if (target->dn) free(target->dn);
	if (target->tr) free(target->tr);
}

void deleteTorDB(torDB *target)
{
	target->cat.clear();
	for (uint64_t i = 0; i < target->pak.size(); ++i) deletePack(&(target->pak[i]));
	target->pak.clear();
}
