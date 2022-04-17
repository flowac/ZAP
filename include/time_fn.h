#ifndef _TIME_FN_H
#define _TIME_FN_H

#include <time.h>

struct tm *get_loc_time();

/**
 * @brief Start LCFS stack timer function
 */
void start_timer(void);

/**
 * @brief Stop LCFS stack timer function
 *
 * @return Elapsed time in microseconds
 */
struct timespec stop_timer(void);

/**
 * @brief Print elapsed time
 */
void print_elapsed_time(void);

#endif // _TIME_FN_H
