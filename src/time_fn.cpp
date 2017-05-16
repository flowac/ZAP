#include "time_fn.h"

#include <time.h>

struct tm *get_loc_time()
{
    time_t raw;
    struct tm *loc_time;

    time(&raw);
    loc_time = localtime(&raw);

    return loc_time;
}

