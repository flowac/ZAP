/* GPL stuff here idk what to put @flowingwater ill let you
 * handle this 
 */
#ifndef _LOG_H
#define _LOG_H

/* This function will append to the log file
 * INPUT:
 * char *msg - message to be printed, can be formatted with %s etc
 * RETURN:
 * 0 - failure
 * 1 - success
 */
int log_msg(char const *msg, ...);

#endif // _LOG_H
