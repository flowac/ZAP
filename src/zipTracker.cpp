#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alib.h"

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
	uint8_t buf[MAGNET_TR_LEN];
	uint32_t ret = 0, len = 0, i, j, mark;
	if (!(len = u8len(tr))) return 0;

	for (i = 0; i < len; ++i, ++ret)
	{
		if (ret >= MAGNET_TR_LEN) [[unlikely]] return 0;
		buf[ret] = tr[i];
		if (tr[i] == '%')
		{
			buf[ret] = TZERO;
			for (mark = ret; i < len && tr[i] == '%'; i += 3)
			{
				if (i + 2 >= len) return 0;
				memcpy(buf3, tr + i + 1, 2);

				buf[mark]++;
				if (buf[mark] > MAX_U8 - TMAX) return 0;
				errno = 0;
				buf[++ret] = (uint8_t) strtol(buf3, NULL, 16);
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
					buf[ret] = MAX_U8 - j;
					i += mark - 1;
					break;
				}
			}
		}
	}
	memcpy(tr, buf, ret);
	tr[ret] = 0;

	return ret;
}

static inline bool reallocTracker(char **ptr, uint32_t len, uint32_t *size)
{
	if (len + TMAX >= *size) [[unlikely]]
	{
		*size += MAX_U8;
		if (!(*ptr = (char *) realloc(*ptr, *size))) return false;
	}
	return true;
}

uint32_t decompressTracker(uint8_t *tr, char **ret)
{
	int i, j;
	uint32_t retLen = 0, allocLen = 0;

	for (i = 0, *ret = NULL; tr[i]; ++i)
	{
		if (!reallocTracker(ret, retLen, &allocLen)) return 0;
		if (tr[i] < TZERO)
			(*ret)[retLen++] = tr[i];
		else if (tr[i] <= MAX_U8 - TMAX)
		{
			for (j = tr[i++] - TZERO; j > 0; ++i, --j)
			{
				retLen += sprintf(*ret + retLen, "%%%02X", tr[i]);
				if (!reallocTracker(ret, retLen, &allocLen)) return 0;
			}
			--i;
		}
		else
			retLen += sprintf(*ret + retLen, tlist[MAX_U8 - tr[i]]);
	}
	ret[retLen] = 0;
	return retLen;
}
