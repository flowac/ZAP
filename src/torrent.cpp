#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "main_lib.h"
#include "torrent_types.h"

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
	if (!target) return;
	for (uint32_t i = 1; i < KEYWORDS_LIM[0]; ++i)
	{
		printf("%s: ", KEYWORDS_R[0][i]);
		for (uint32_t j = 1; j < KEYWORDS_LIM[i]; ++j)
		{
			printf("\n\t%s: ", KEYWORDS_R[i][j]);
			for (uint64_t k = 0; k < std::min((uint64_t) 10, (uint64_t) target->cat[i][j].size()); ++k)
				printf("%u ", target->cat[i][j][k]);
		}
		printf("\n");
	}
}

void printTorWordMap(torDB *target)
{
	if (!target) return;
	printf("Torrent word map");
	for (std::pair<uint32_t, std::vector<uint32_t>> i : target->wrd)
	{
		printf("\n%8u: ", i.first);
		for (uint32_t j : i.second) printf("%u ", j);
	}
	printf("\n");
}

bool processPack(torDB *td, pack *px)
{
	uint8_t kt1, kt2, kt[MAGNET_KT_LEN];
	uint32_t idx, i, j, st[MAGNET_ST_LEN];
	if (!td || !px) return false;
	convertKeyword(px->kt, &kt1, &kt2);
	if (!isKeywordValid(kt1, kt2)) return false;
	idx = td->pak.size();

	td->cat[kt1][kt2 ? kt2 : 1].push_back(idx);
	if (!encodeMsg(px->dn, st, px->ut, kt)) return false;

	for (i = 0; i < MAGNET_ST_LEN && (j = st[i]); ++i)
	{
		if (td->wrd.contains(j)) td->wrd[j].push_back(idx);
		else
		{
			std::vector<uint32_t> tmp;
			tmp.push_back(idx);
			td->wrd[j] = tmp;
		}
	}

	for (i = 0; i < MAGNET_KT_LEN && (j = kt[i]) < MAGNET_NUM_LEN; ++i) td->num[j].push_back(idx);
	return true;
}

bool newPack(torDB *td, uint8_t xt[MAGNET_XT_LEN], uint64_t xl, char *dn, uint8_t *tr, uint8_t kt, uint8_t crc[SHAKE_LEN])
{
	uint8_t kt1, kt2;
	uint32_t ndn = 0, ntr = 0;
	pack px = {.crc = {0}, .xt = {0}, .xl = 0ULL, .dn = NULL, .tr = NULL, .ut = NULL, .kt = 0};
	if (!td || !xt || !dn || !tr) goto cleanup;

	if (!(ndn = strlen(dn)) || ndn > MAGNET_DN_LEN) goto cleanup;
	if (!(ntr = u8len(tr)) || ntr > MAGNET_TR_LEN) goto cleanup;
	if (!(ntr = compressTracker(tr))) goto cleanup;

	if (crc) memcpy(px.crc, crc, SHAKE_LEN);
	memcpy(px.xt, xt, MAGNET_XT_LEN);
	px.xl = xl;
	if (!(px.dn = (char *) calloc(ndn + 1, 1))) goto cleanup;
	memcpy(px.dn, dn, ndn);
	if (!(px.tr = (uint8_t *) calloc(ntr + 1, 1))) goto cleanup;
	memcpy(px.tr, tr, ntr);

	convertKeyword(kt, &kt1, &kt2);
	if (!isKeywordValid(kt1, kt2)) goto cleanup;
	px.kt = kt;

	if (!processPack(td, &px))
	{
		printf("proc pack failed %u %u\n", kt & MAX_U4, kt >> 4);
		goto cleanup;
	}
	if (!checkPack(&px, !crc)) goto cleanup;
	td->pak.push_back(px);

	return true;
cleanup:
	printf("\nfailed new pack kt[i] %s< ndn %u ntr %u %p kt: %u %u\n",
		   dn, ndn, ntr, tr, kt & MAX_U4, kt >> 4);
	if (px.dn) free(px.dn);
	if (px.tr) free(px.tr);
	return false;
}

void convertKeyword(uint8_t kt, uint8_t *kt1, uint8_t *kt2)
{
	if (kt1) *kt1 = kt & MAX_U4;
	if (kt2) *kt2 = kt >> 4;
}

const char *getKeyword1(uint8_t kt1)
{
	if (isKeywordValid(kt1, (uint8_t) 0)) return KEYWORDS_R[0][kt1];
	return NULL;
}

const char *getKeyword2(uint8_t kt1, uint8_t kt2)
{
	if (isKeywordValid(kt1, kt2)) return KEYWORDS_R[kt1][kt2];
	return NULL;
}

bool isKeywordValid(uint8_t kt1, uint8_t kt2)
{
	if (kt1 == 0 || kt1 >= KEYWORDS_LIM[0]) return false;
	if (kt2 >= KEYWORDS_LIM[kt1]) return false;
	return true;
}

uint8_t lookupKeyword(const char *kt1, const char *kt2)
{
	uint8_t ret = 0;
	std::map<const char *, uint8_t>::iterator it;

	if (!kt1) return ret;
	if ((it = KEYWORDS[0].find(kt1)) == KEYWORDS[0].end()) return ret;
	ret = it->second & MAX_U4;

	if (!kt2) return ret;
	if ((it = KEYWORDS[ret].find(kt2)) == KEYWORDS[ret].end()) return ret;
	ret |= (it->second & MAX_U4) << 4;

	return ret;
}

static bool freqVecCmp(std::pair<uint32_t, uint8_t> &a, std::pair<uint32_t, uint8_t> &b)
{
	return a.second < b.second;
}

std::vector<uint32_t> searchTorDB(torDB *td, uint8_t kt1, uint8_t kt2, const char *str)
{
	bool checkKT = isKeywordValid(kt1, kt2);
	char *tmpc, *ut = NULL, utv[MAGNET_UT_CNT][MAGNET_UT_LEN];
	uint8_t kt[MAGNET_KT_LEN];
	uint32_t i, j, st[MAGNET_ST_LEN];
	std::vector<uint32_t> result;
	std::vector<std::pair<uint32_t, uint8_t>> freqVec;
	// TODO: make the map atomic and do multi thread st kt ut search
	std::map<uint32_t, uint8_t> freqMap; // result frequency map
	if (!td) return result;

	if (checkKT)
	{
		if (kt2) for (uint32_t k : td->cat[kt1][kt2]) freqMap[k] = 0;
		else for (std::vector<uint32_t> x : td->cat[kt1]) for (uint32_t k : x) freqMap[k] = 0;
	}

	encodeMsg(str, st, ut, kt);
	memset(utv, 0, MAGNET_UT_CNT * MAGNET_UT_LEN);
	// process and search non-dictionary words
	if (ut)
	{
		for (tmpc = strtok(ut, " "), i = 0; i < MAGNET_UT_CNT && tmpc; tmpc = strtok(NULL, " "))
		{
			if (strlen(tmpc) >= MAGNET_UT_LEN) continue;
			strcpy(utv[i], tmpc);
		}
		free(ut);

		for (uint32_t k = 0; k < td->pak.size(); ++k)
		{
			pack &px = td->pak[k];
			if (!px.ut) continue;

			for (i = 0; i < MAGNET_UT_CNT && utv[i][0]; ++i)
			{
				if (!strstr(px.ut, utv[i])) continue;
				if (freqMap.contains(k)) freqMap[k] += 3;
				else if (!checkKT) freqMap[k] = 3;
			}
		}
	}

	// search with numbers
	for (i = 1; i < MAGNET_KT_LEN && (j = kt[i]) < MAGNET_NUM_LEN; ++i)
	{
		for (uint32_t k : td->num[j])
		{
			if (freqMap.contains(k)) freqMap[k] += 1;
			else if (!checkKT) freqMap[k] = 1;
		}
	}

	// search with dictionary words
	for (i = 0; (j = st[i]); ++i)
	{
		if (!td->wrd.contains(j)) continue;
		for (uint32_t k : td->wrd[j])
		{
			if (freqMap.contains(k)) freqMap[k] += 3;
			else if (!checkKT) freqMap[k] = 3;
		}
	}

	for (std::pair<uint32_t, uint8_t> x : freqMap) freqVec.push_back(x);
	std::sort(freqVec.begin(), freqVec.end(), freqVecCmp);
	for (std::pair<uint32_t, uint8_t> x : freqVec) if (x.second > 2) result.insert(result.begin(), x.first);

//	printf("freqMap:\n");\
	for (std::pair<uint32_t, uint8_t> x : freqVec) if (x.second > 2) printf("%u %u\n", x.first, x.second);

	return result;
}

std::vector<uint32_t> searchTorDB(torDB *td, const char *kt1, const char *kt2, const char *str)
{
	uint8_t kt1u, kt2u;

	kt1u = lookupKeyword(kt1, kt2);
	kt2u = kt1u >> 4;
	kt1u &= MAX_U4;
	return searchTorDB(td, kt1u, kt2u, str);
}

static inline void deletePack(pack *target)
{
	if (!target) return;
	if (target->dn) free(target->dn);
	if (target->tr) free(target->tr);
	if (target->ut) free(target->ut);
}

torDB::torDB()
{
	cat.resize(KEYWORDS_LIM[0]);
	for (uint8_t i = 1; i < KEYWORDS_LIM[0]; ++i) cat[i].resize(KEYWORDS_LIM[i]);
}

torDB::~torDB()
{
	for (uint64_t i = 0; i < pak.size(); ++i) deletePack(&(pak[i]));
}
