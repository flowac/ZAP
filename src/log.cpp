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

bool log_msg(char const *msg, ...)
{
	FILE *fd = logDescriptor::getFD();
	struct tm *local_time_t = get_loc_time();
	char buffer_msg[512];
	char buffer_time[64];
	va_list msg_formatted;
	if (!fd || !local_time_t)
	{
		printf("LOG INIT FAILED: log file descriptor = %p, local time = %p\n", fd, local_time_t);
		return false;
	}

	va_start(msg_formatted, msg);	// convert args to string
	strftime(buffer_time, sizeof(buffer_time), "%a %b %T", local_time_t);
	vsnprintf(buffer_msg, sizeof(buffer_msg), msg, msg_formatted);

	fprintf(fd, "%s %s\n", buffer_time, buffer_msg);
	va_end(msg_formatted);
	return true;
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

