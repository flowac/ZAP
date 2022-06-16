#ifndef _TORRENT_TYPES_H
#define _TORRENT_TYPES_H

#include "types.h"

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

typedef struct
{
	bool operator()(const char *left, const char *right) const
	{
		return strcmp(left, right) < 0;
	}
} CStringComp;

std::map<const char *, uint8_t, CStringComp> KEYWORDS[] = {
	// Keyword 1
	{
		{"Other",        1},
		{"Applications", 2},
		{"Audio",        3},
		{"Games",        4},
		{"Porn",         5},
		{"Video",        6}
	},
	// Keyword 2
	{
		{"Other",     1},
		{"E-books",   2},
		{"Comics",    3},
		{"Pictures",  4},
		{"Covers",    5},
		{"Physibles", 6}
	},
	{
		{"Other",    1},
		{"Windows",  2},
		{"Mac",      3},
		{"UNIX",     4},
		{"Handheld", 5},
		{"IOS",      6},
		{"Android",  7}
	},
	{
		{"Other",       1},
		{"Music",       2},
		{"Audio books", 3},
		{"Sound clips", 4},
		{"FLAC",        5}
	},
	{
		{"Other",    1},
		{"PC",       2},
		{"Mac",      3},
		{"PSx",      4},
		{"XBOX",     5},
		{"Wii",      6},
		{"Handheld", 7},
		{"IOS",      8},
		{"Android",  9}
	},
	{
		{"Other",       1},
		{"Movies",      2},
		{"Pictures",    3},
		{"Games",       4},
		{"HD - Movies", 5},
		{"Movie clips", 6}
	},
	{
		{"Other",         1},
		{"Movies",        2},
		{"Music videos",  3},
		{"Movie clips",   4},
		{"TV shows",      5},
		{"Handheld",      6},
		{"HD - Movies",   7},
		{"HD - TV shows", 8},
		{"3D",            9}
	}
};

constexpr uint8_t KEYWORDS_LIM[] = {
	7,
	7,
	8,
	6,
	10,
	7,
	10
};

// Reverse keyword lookup
const char *KEYWORDS_R0[] = {
	NULL,
	"Other",
	"Applications",
	"Audio",
	"Games",
	"Porn",
	"Video"
};
const char *KEYWORDS_R1[] = {
	NULL,
	"Other",
	"E-books",
	"Comics",
	"Pictures",
	"Covers",
	"Physibles"
};
const char *KEYWORDS_R2[] = {
	NULL,
	"Other",
	"Windows",
	"Mac",
	"UNIX",
	"Handheld",
	"IOS",
	"Andriod"
};
const char *KEYWORDS_R3[] = {
	NULL,
	"Other",
	"Music",
	"Audio books",
	"Sound clips",
	"FLAC"
};
const char *KEYWORDS_R4[] = {
	NULL,
	"Other",
	"PC",
	"Mac",
	"PSx",
	"XBOX",
	"Wii",
	"Handheld",
	"IOS",
	"Android",
};
const char *KEYWORDS_R5[] = {
	NULL,
	"Other",
	"Movies",
	"Pictures",
	"Games",
	"HD - Movies",
	"Movie clips"
};
const char *KEYWORDS_R6[] = {
	NULL,
	"Other",
	"Movies",
	"Music videos",
	"Movie clips",
	"TV shows",
	"Handheld",
	"HD - Movies",
	"HD - TV shows",
	"3D"
};

const char **KEYWORDS_R[] = {
	// Keyword 1
	KEYWORDS_R0,
	// Keyword 2
	KEYWORDS_R1,
	KEYWORDS_R2,
	KEYWORDS_R3,
	KEYWORDS_R4,
	KEYWORDS_R5,
	KEYWORDS_R6,
};

#endif //_TORRENT_TYPES_H
