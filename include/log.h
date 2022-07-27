/**
 * @file log.h
 * @brief Functions that deal with Logging
 */

#ifndef _LOG_H
#define _LOG_H

#include <fstream>

#include <cerrno>
#include <cstdio>
#include <cstring>

// TODO: Remove this autistic custom uint64_t definition
#include "7zTypes.h"

#define LOG_PATH "log"

class logDescriptor {
public:
	static FILE *getFD()
	{
		static logDescriptor L;
		return L.fd;
	}
private:
	FILE *fd = NULL;
	logDescriptor () {if (!(fd = fopen(LOG_PATH, "a"))) fd = stderr;}
	~logDescriptor () {if (fd != stderr) fclose(fd);}
};

/**
 * @brief This function will append to the log file
 *        Please use the macro log() below instead of this
 */
void log_msg(const char *file, const char *function, const int line, const char *format, ...);
#define log(...) log_msg(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/**
 * @brief Print pass or fail of test cases to stdout
 */
bool pstat(bool status, const char *msg);

/**
 * @brief Get the size of a file
 * 
 * @return Size of file (long value)
 */
long get_file_size_c(FILE *fd);//!< Fd to the file

/**
 * @brief This function will return the size of the file specified
 * by f_stream
 *
 * This function is not used and probably not working will remove
 * later
 * @return Size of file on success \n
 * 0 on failure
 */
UInt64 get_file_size(std::ofstream f_stream);//!< Filestream to a file

#endif // _LOG_H

