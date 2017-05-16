/* Flowingwater slap a liscence
 */

#include "log.h"
#include "time_fn.h"

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

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
