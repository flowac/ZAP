/* Flowingwater slap a liscence
 */
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ostream>
#include <iostream>
#include <fstream>

#include "log.h"
#include "time_fn.h"
/* extern */
#include "C/7zTypes.h"

int log_msg(char const * msg, ...)
{
    /*@flowing water do we want to open the fd every time this fn is called?*/
    struct tm *local_time_t = get_loc_time();
    if (local_time_t == NULL) {
        printf("LOG: Failed to get local time\n");
        return 0;
    }
    char buffer_msg[512];
    /* perhaps switch to boost so we dont need this buff */
    char buffer_time[64];
    va_list msg_formatted;

    FILE * fd =  fopen("log", "a"); // fd to log file
    if (fd == NULL) {
        printf("LOG: Failed to open logfile: %s\n",
               strerror(errno));
        return 0;
    }

    va_start(msg_formatted, msg); // convert args to string
    strftime(buffer_time, sizeof(buffer_time),
             "%a %b %T", local_time_t);
    vsnprintf(buffer_msg, sizeof(buffer_msg),
              msg, msg_formatted);

    fprintf(fd, "%s %s\n", buffer_time, buffer_msg);

    if (fd)
        fclose(fd);
    va_end(msg_formatted);
    return 1;
}

UInt64 get_file_size(std::ofstream f_stream)
{
    UInt64 size_u64 = 0;

    std::char_traits<char>::pos_type file_start = f_stream.tellp(); // get start pos
    f_stream.seekp(0, std::ios_base::end); // goto end and get pos
    std::char_traits<char>::pos_type file_end = f_stream.tellp();
    size_u64 = (UInt64)(file_end - file_start);
    f_stream.seekp(0, std::ios_base::beg);

    return size_u64;
}
