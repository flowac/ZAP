#ifndef _TIME_FN_H
#define _TIME_FN_H

#include <time.h>

/**
 * @brief Return current time
 *        Nano seconds stored in lower 30 bits
 *        Seconds since epoch in upper 34 bits
 */
uint64_t nsNow(void);

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
