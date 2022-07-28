#include <ostream>
#include <iostream>
#include <fstream>

#include <ctime>
#include <cstdarg>

#include "log.h"
#include "time_fn.h"

bool pstat(bool status, const char *msg)
{
	printf("[%s] %s\n", status ? "PASS" : "FAIL", msg);
	return status;
}

void log_message(const char *file, const char *function, const int line, const char *format, ...)
{
	char buf32[32];
	char buf256[256];
	static FILE *fd = logDescriptor::getFD();
	time_t raw;
	va_list arg;

	time(&raw);
	strftime(buf32, sizeof(buf32), "%F %T", localtime(&raw));
	snprintf(buf256, sizeof(buf256), "%s %s %s:%d %s\n", buf32, file, function, line, format);

	va_start(arg, format);
	vfprintf(fd, buf256, arg);
	va_end(arg);
}

void note_message(const char *file, const char *function, const int line, const char *format, ...)
{
	char buf256[256];
	va_list arg;

	snprintf(buf256, sizeof(buf256), "%s %s:%d %s\n", file, function, line, format);

	va_start(arg, format);
	vprintf(buf256, arg);
	va_end(arg);
}

long get_file_size_c(FILE *fd)
{
	fseek(fd, 0, SEEK_END);
	long size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	return size;
}

UInt64 get_file_size(std::ofstream f_stream)
{
	UInt64 size_u64 = 0;

	std::char_traits < char >::pos_type file_start = f_stream.tellp();	// get start pos
	f_stream.seekp(0, std::ios_base::end);	// goto end and get pos
	std::char_traits < char >::pos_type file_end = f_stream.tellp();
	size_u64 = (UInt64) (file_end - file_start);
	f_stream.seekp(0, std::ios_base::beg);

	return size_u64;
}

