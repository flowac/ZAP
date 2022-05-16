/**
 * @file log.h
 * @brief Functions that deal with Logging
 */

#ifndef _LOG_H
#define _LOG_H

#include <fstream>
#include "7zTypes.h"

#include <string.h>
#include <errno.h>

/**
 * @brief Log the the strerror of the errno
 */
#define log_msg_default log_msg("%s %s:%d: %s\n", __FILE__, __FUNCTION__, __LINE__, strerror(errno))

/**
 * @brief Log the string passed to the macro
 */
#define log_msg_custom(msg) log_msg("%s %s:%d: %s\n", __FILE__, __FUNCTION__, __LINE__, msg)

/**
 * @brief Log the string passed to the macro and the strerror
 * of the errno
 */
#define log_msg_custom_errno(msg, x) log_msg("%s %s:%d: %s:%d\n", __FILE__, __FUNCTION__, __LINE__, msg, x)

/**
 * @brief Print pass or fail of test cases to stdout
 */
bool pstat(bool status, const char *msg);

/**
 * @brief De-initialize logger
 */
void log_deinit(void);

/**
 * @brief This function will append to the log file
 *
 * @return 0 - failure\n
 * 1 - success
 */
int log_msg(char const *msg,//!< Format specified string
            ...);           //!< Args if any

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

