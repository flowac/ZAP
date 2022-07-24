/* Word processing functions
   stemmer by:
   tartarus.org/martin/PortStemmer/
   ANSI C thread safe
   Word list by:
   gwicks.net/justwords.html
 */
#include <ctype.h>   /* for isalnum, tolower */
#include <stdlib.h>  /* for malloc, free */
#include <string.h>  /* for memcmp, memmove */

#include "main_lib.h"

typedef struct {
    char *b;           /*buffer for word to be stemmed */
    int k;             /* offset to the end of the string */
    int j;             /* a general offset into the string */
} stem;

#define TRUE 1
#define FALSE 0

static int cons(stem *z, int i)
{
    switch (z->b[i])
    {
    case 'a': case 'e': case 'i': case 'o': case 'u':
        return FALSE;
    case 'y':
        return (i == 0) ? TRUE : !cons(z, i - 1);
    default:
        return TRUE;
    }
}

static int m(stem *z)
{
    int n = 0;
    int i = 0;
    int j = z->j;
    while(TRUE)
    {
        if (i > j) return n;
        if (!cons(z, i)) break;
        i++;
    }
    i++;
    while(TRUE)
    {
        while(TRUE)
        {
            if (i > j) return n;
            if (cons(z, i)) break;
            i++;
        }
        i++;
        n++;
        while(TRUE)
        {
            if (i > j) return n;
            if (!cons(z, i)) break;
            i++;
        }
        i++;
    }
}

static int vowelinstem(stem *z)
{
    int j = z->j;
    int i; for (i = 0; i <= j; i++) if (!cons(z, i)) return TRUE;
    return FALSE;
}

static int doublec(stem *z, int j)
{
    char *b = z->b;
    if (j < 1) return FALSE;
    if (b[j] != b[j - 1]) return FALSE;
    return cons(z, j);
}

static int cvc(stem *z, int i)
{
    if (i < 2 || !cons(z, i) || cons(z, i - 1) || !cons(z, i - 2)) return FALSE;
    {
        int ch = z->b[i];
        if (ch  == 'w' || ch == 'x' || ch == 'y') return FALSE;
    }
    return TRUE;
}

static int ends(stem *z, const char *s)
{
    int length = s[0];
    char *b = z->b;
    int k = z->k;
    if (s[length] != b[k]) return FALSE; /* tiny speed-up */
    if (length > k + 1) return FALSE;
    if (memcmp(b + k - length + 1, s + 1, length) != 0) return FALSE;
    z->j = k-length;
    return TRUE;
}

static void setto(stem *z, const char *s)
{
    int length = s[0];
    int j = z->j;
    memmove(z->b + j + 1, s + 1, length);
    z->k = j+length;
}

static void r(stem *z, const char *s)
{
    if (m(z) > 0) setto(z, s);
}

static void step1ab(stem *z)
{
    char *b = z->b;
    if (b[z->k] == 's')
    {
        if (ends(z, "\04" "sses")) z->k -= 2;
        else if (ends(z, "\03" "ies")) setto(z, "\01" "i");
        else if (b[z->k - 1] != 's') z->k--;
    }
    if (ends(z, "\03" "eed"))
    {
        if (m(z) > 0) z->k--;
    }
    else if ((ends(z, "\02" "ed") || ends(z, "\03" "ing")) && vowelinstem(z))
    {
        z->k = z->j;
        if (ends(z, "\02" "at")) setto(z, "\03" "ate"); else
        if (ends(z, "\02" "bl")) setto(z, "\03" "ble"); else
        if (ends(z, "\02" "iz")) setto(z, "\03" "ize"); else
        if (doublec(z, z->k))
        {
            z->k--;
            {
                int ch = b[z->k];
                if (ch == 'l' || ch == 's' || ch == 'z') z->k++;
            }
        }
        else if (m(z) == 1 && cvc(z, z->k)) setto(z, "\01" "e");
    }
}

static void step1c(stem *z)
{
    if (ends(z, "\01" "y") && vowelinstem(z)) z->b[z->k] = 'i';
}

static void step2(stem *z) {
    switch (z->b[z->k-1])
    {
    case 'a':
        if (ends(z, "\07" "ational")) { r(z, "\03" "ate"); break; }
        if (ends(z, "\06" "tional")) { r(z, "\04" "tion"); break; }
        break;
    case 'c':
        if (ends(z, "\04" "enci")) { r(z, "\04" "ence"); break; }
        if (ends(z, "\04" "anci")) { r(z, "\04" "ance"); break; }
        break;
    case 'e':
        if (ends(z, "\04" "izer")) { r(z, "\03" "ize"); break; }
        break;
    case 'l':
        if (ends(z, "\03" "bli")) { r(z, "\03" "ble"); break; } /*-DEPARTURE-*/
        if (ends(z, "\04" "alli")) { r(z, "\02" "al"); break; }
        if (ends(z, "\05" "entli")) { r(z, "\03" "ent"); break; }
        if (ends(z, "\03" "eli")) { r(z, "\01" "e"); break; }
        if (ends(z, "\05" "ousli")) { r(z, "\03" "ous"); break; }
        break;
    case 'o':
        if (ends(z, "\07" "ization")) { r(z, "\03" "ize"); break; }
        if (ends(z, "\05" "ation")) { r(z, "\03" "ate"); break; }
        if (ends(z, "\04" "ator")) { r(z, "\03" "ate"); break; }
        break;
    case 's':
        if (ends(z, "\05" "alism")) { r(z, "\02" "al"); break; }
        if (ends(z, "\07" "iveness")) { r(z, "\03" "ive"); break; }
        if (ends(z, "\07" "fulness")) { r(z, "\03" "ful"); break; }
        if (ends(z, "\07" "ousness")) { r(z, "\03" "ous"); break; }
        break;
    case 't':
        if (ends(z, "\05" "aliti")) { r(z, "\02" "al"); break; }
        if (ends(z, "\05" "iviti")) { r(z, "\03" "ive"); break; }
        if (ends(z, "\06" "biliti")) { r(z, "\03" "ble"); break; }
        break;
    case 'g':
        if (ends(z, "\04" "logi")) { r(z, "\03" "log"); break; } /*-DEPARTURE-*/
    }
}

static void step3(stem *z) {
    switch (z->b[z->k])
    {
    case 'e':
        if (ends(z, "\05" "icate")) { r(z, "\02" "ic"); break; }
        if (ends(z, "\05" "ative")) { r(z, "\00" ""); break; }
        if (ends(z, "\05" "alize")) { r(z, "\02" "al"); break; }
        break;
    case 'i':
        if (ends(z, "\05" "iciti")) { r(z, "\02" "ic"); break; }
        break;
    case 'l':
        if (ends(z, "\04" "ical")) { r(z, "\02" "ic"); break; }
        if (ends(z, "\03" "ful")) { r(z, "\00" ""); break; }
        break;
    case 's':
        if (ends(z, "\04" "ness")) { r(z, "\00" ""); break; }
        break;
    }
}

static void step4(stem *z)
{
    switch (z->b[z->k-1])
    {
    case 'a':
        if (ends(z, "\02" "al")) break;
        return;
    case 'c':
        if (ends(z, "\04" "ance")) break;
        if (ends(z, "\04" "ence")) break;
        return;
    case 'e':
        if (ends(z, "\02" "er")) break;
        return;
    case 'i':
        if (ends(z, "\02" "ic")) break;
        return;
    case 'l':
        if (ends(z, "\04" "able")) break;
        if (ends(z, "\04" "ible")) break;
        return;
    case 'n':
        if (ends(z, "\03" "ant")) break;
        if (ends(z, "\05" "ement")) break;
        if (ends(z, "\04" "ment")) break;
        if (ends(z, "\03" "ent")) break;
        return;
    case 'o':
        if (ends(z, "\03" "ion") && z->j >= 0 && (z->b[z->j] == 's' || z->b[z->j] == 't')) break;
        if (ends(z, "\02" "ou")) break;
        return;
        /* takes care of -ous */
    case 's':
        if (ends(z, "\03" "ism")) break;
        return;
    case 't':
        if (ends(z, "\03" "ate")) break;
        if (ends(z, "\03" "iti")) break;
        return;
    case 'u':
        if (ends(z, "\03" "ous")) break;
        return;
    case 'v':
        if (ends(z, "\03" "ive")) break;
        return;
    case 'z':
        if (ends(z, "\03" "ize")) break;
        return;
    default:
        return;
    }
    if (m(z) > 1) z->k = z->j;
}

static void step5(stem *z)
{
    char *b = z->b;
    z->j = z->k;
    if (b[z->k] == 'e')
    {
        int a = m(z);
        if (a > 1 || (a == 1 && !cvc(z, z->k - 1))) z->k--;
    }
    if (b[z->k] == 'l' && doublec(z, z->k) && m(z) > 1) z->k--;
}

void filterLine(char *b, int *k)
{
	char prev = 0;
    for (int i = 0; i < *k;)
    {
        if (!isalnum(b[i]))
        {
			if (prev != ' ')
			{
				if (b[i] == '-' || b[i] == '.')
				{
					prev = b[i] = ' ';
					++i;
				}
				else if (b[i] == ' ')
				{
					prev = ' ';
					++i;
				}
				else
				{
					goto LINE_MOVE;
				}
			}
			else
			{
LINE_MOVE:
				memmove(b + i, b + i + 1, *k - i);
				--(*k);
			}
        }
        else
        {
            prev = b[i] = tolower(b[i]);
            ++i;
        }
    }
}

void stemWord(char *buf, int *len)
{
    stem z = {.b = buf, .k = *len - 1, .j = 0};
    if (z.k <= 1 || !buf) return;

    step1ab(&z);
    if (z.k > 0)
    {
        step1c(&z); step2(&z); step3(&z); step4(&z); step5(&z);
    }
    *len = ++z.k;
    buf[z.k] = 0;
}

bool encodeMsg(const char *msg, uint32_t st[MAGNET_ST_LEN], char *&ut, uint8_t kt[MAGNET_KT_LEN])
{
	char buf[BUF1K];
	char *p, *s;
	int ilen, klen = 0;
	uint32_t ret = 0;
	uint32_t blen, ulen = 0, ucnt = 0;
	uint64_t idx;

	if (!msg || !st || !kt) return false;
	ut = NULL;
	memset(st, 0, sizeof(uint32_t) * MAGNET_ST_LEN);
	memset(kt, MAX_U8, MAGNET_KT_LEN);

	if ((ilen = strlen(msg)) >= (int) BUF1K) return false;
	memcpy(buf, msg, ilen + 1);
	filterLine(buf, &ilen);

	for (p = buf; (s = p); (p && *(p + 1) > 0) ? ++p : p = NULL)
	{
		if ((p = strchr(s, ' ')))
		{
			*p = 0;
			ilen = p - s;
		}
		else ilen = strlen(s);

		stemWord(s, &ilen);
		if (STOPWORDS_EN.find(s)) continue;
		if ((idx = (uint64_t) WORDS_EN.find(s)))
		{
			if (ret < MAGNET_ST_LEN) st[ret++] = idx;
		}
		else
		{
			if (ucnt > MAGNET_UT_CNT || ilen > MAGNET_UT_LEN) continue;
			for (char *tmp = s; *tmp && klen < MAGNET_KT_LEN; ++tmp)
			{
				if (!isdigit(*tmp)) continue;
				idx = *tmp - '0';
				if (isdigit(*(tmp + 1))) idx = idx * 10 + *(++tmp) - '0';
				kt[klen++] = idx;
			}

			blen = ilen + ulen + 1;
			if (blen > MAGNET_UT_MAX) continue;
			ut = (char *) realloc(ut, blen);
			memcpy(ut + ulen, s, ilen);
			ulen = blen;
			ucnt++;
			ut[--blen] = ' ';
		}
	}

	if (ut) (ut)[--ulen] = 0;
	return true;
}
