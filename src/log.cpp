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

void log_msg(char const *format, ...)
{
	char buf64[64];
	static FILE *fd = logDescriptor::getFD();
	struct tm *local_time_t = get_loc_time();
	va_list arg;

	if (local_time_t)
	{
		strftime(buf64, sizeof(buf64), "%F %T ", local_time_t);
		fprintf(fd, buf64);
	}

	va_start(arg, format);
	vfprintf(fd, format, arg);
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

