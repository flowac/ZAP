#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alib.h"

typedef enum
{
	TR_ZERO  = 1 << 7,
	TR_START = MAX_U8,
	TR_UDP   = MAX_U8 - 1,
	TR_HTTP  = MAX_U8 - 2,
	TR_ANN   = MAX_U8 - 3,
	TR_TOR   = MAX_U8 - 4,
	TR_TRA   = MAX_U8 - 5,
	TR_COM   = MAX_U8 - 6,
	TR_ORG   = MAX_U8 - 7,
	TR_OPEN  = MAX_U8 - 8,
	TR_MAX   = MAX_U8 - 9
} TRACKER_ESCAPE;

uint32_t compressTracker(uint8_t *tr)
{
	char buf3[3] = {0, 0, 0};
	uint8_t buf[MAGNET_TR_LEN];
	uint32_t ret = 0, len = 0, i, mark;
	if (!tr || !(len = strlen(tr))) return 0;

	for (i = 0; i < len; ++i, ++ret)
	{
		buf[ret] = tr[i];
		if (tr[i] == 'u' && (i + 2) < len && memcmp(tr + i, "udp", 3) == 0)
		{
			buf[ret] = TR_UDP;
			i += 2;
		}
		else if (tr[i] == '&' && (i + 3) < len && memcmp(tr + i, "&tr=", 4) == 0)
		{
			buf[ret] = TR_START;
			i += 3;
		}
		else if (tr[i] == '%')
		{
			buf[ret] = TR_ZERO;
			for (mark = ret; i < len && tr[i] == '%'; i += 3)
			{
				if (i + 2 >= len) return 0;
				memcpy(buf3, tr + i + 1, 2);

				buf[mark] += 1;
				if (buf[mark] > TR_MAX) return 0;
				errno = 0;
				buf[++ret] = (uint8_t) strtol(buf3, NULL, 16);
				if (errno != 0) return 0;
			}
			--i;
		}
		else if (tr[i] == 'a' && (i + 7) < len && memcmp(tr + i, "announce", 8) == 0)
		{
			buf[ret] = TR_ANN;
			i += 7;
		}
		else if (tr[i] == 'h' && (i + 3) < len && memcmp(tr + i, "http", 4) == 0)
		{
			buf[ret] = TR_HTTP;
			i += 3;
		}
		else if (tr[i] == 'o' && (i + 3) < len && memcmp(tr + i, "open", 4) == 0)
		{
			buf[ret] = TR_OPEN;
			i += 3;
		}
		else if (tr[i] == 't' && (i + 6) < len)
		{
			if (memcmp(tr + i, "torrent", 7) == 0)
			{
				buf[ret] = TR_TOR;
				i += 6;
			}
			else if (memcmp(tr + i, "tracker", 7) == 0)
			{
				buf[ret] = TR_TRA;
				i += 6;
			}
		}
		else if (tr[i] == '.' && (i + 3) < len)
		{
			if (memcmp(tr + i, ".com", 4) == 0)
			{
				buf[ret] = TR_COM;
				i += 3;
			}
			else if (memcmp(tr + i, ".org", 4) == 0)
			{
				buf[ret] = TR_ORG;
				i += 3;
			}
		}
	}
	memcpy(tr, buf, ret);
	tr[ret] = 0;

	return ret;
}

uint32_t decompressTracker(uint8_t *tr)
{
	int i, j, trsub;
	for (i = 0; tr[i]; i++)
	{
		trsub = tr[i] - TR_ZERO;
		if (tr[i] < TR_ZERO) printf("%c", tr[i]);
		else
		{
			if (tr[i] <= TR_MAX)
			{
				// escape sequence
				printf("[");
				for (j = 0; j < trsub; j++)
				{
					printf("%02X", tr[j + i + 1]);
				}
				i += trsub;
				printf("]");
			}
			else if (tr[i] == TR_START)
			{
				// next link
				printf("\n>");
			}
			else
			{
				// dictionary lookup
				printf("(%02X)", trsub);
			}
		}
	}
	printf("\n");
	return 0;
}
