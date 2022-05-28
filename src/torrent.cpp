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

bool processPack(torDB *td, pack *px)
{
	uint64_t left, mid, right;
	uint32_t i;
	if (!td || !px) return false;

	return true;
	for (i = 0; i < MAGNET_KT_COUNT; ++i)
	{
	}

	do
	{
		left = 0;
		right = td->pak.size();
		mid = (left + right) >> 1;
	}
	while (left != right);

	return true;
}

bool newPack(torDB *td, uint8_t xt[MAGNET_XT_LEN], uint64_t xl, char *dn, uint8_t *tr, char *kt[MAGNET_KT_COUNT])
{
	uint32_t ndn = 0, ntr = 0, nkt = 0, i = 0;
	pack px = {.crc = {0}, .xt = {0}, .xl = 0ULL, .dn = NULL, .tr = NULL, .kt = {0}};
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

	for (i = 0; i < MAGNET_KT_COUNT; ++i) px.kt[i] = NULL;
	for (i = 0; i < MAGNET_KT_COUNT; ++i)
	{
		if (!kt[i] || 0 == (nkt = strlen(kt[i]))) break;
		if (nkt > MAGNET_KT_LEN) goto cleanup;
		px.kt[i] = kt[i];
	}

	if (!checkPack(&px, true)) goto cleanup;
	td->pak.push_back(px);
	return processPack(td, &px);
cleanup:
	printf("\nfailed new pack kt[i] %u[%u] %s< ndn %u ntr %u %p\n", nkt, i, dn, ndn, ntr, tr);
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
	for (uint32_t i = 0; i < MAGNET_KT_COUNT; ++i) if (target->kt[i]) free(target->kt[i]);
}

void deleteTorDB(torDB *target)
{
	target->cat.clear();
	for (uint64_t i = 0; i < target->pak.size(); ++i) deletePack(&(target->pak[i]));
	target->pak.clear();
}
