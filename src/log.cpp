/* Flowingwater slap a liscence
 */

#include "log.h"
#include <stdio.h>
#include <stdarg.h>

int log_msg(char * msg, ...)
{
    /*@flowing water do we want to open the fd every time this fn is called?*/
    va_list msg_formatted;
    va_start(msg_formatted, msg); // convert args to string
    FILE * fd =  fopen("log", "a"); // fd to log file
    if (fd == NULL) {
        printf("Failed to open LOG ");
        perror("fopen");
        return 0;
    }

    vfprintf(fd, msg, msg_formatted); // print to log

    if (fd)
        fclose(fd);
    va_end(msg_formatted);
    return 1;
}
