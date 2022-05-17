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
