#include "time_fn.h"
#include <stdio.h>
#include <stdlib.h>

static struct timespec *t_stack = NULL;
static unsigned long t_stack_len = 0;

// TODO: Why is this here?
struct tm *get_loc_time()
{
	time_t raw;
	struct tm *loc_time;

	time(&raw);
	loc_time = localtime(&raw);

	return loc_time;
}

void start_timer(void)
{
	++t_stack_len;
	if (!(t_stack = (struct timespec *) realloc(t_stack, sizeof(struct timespec) * t_stack_len)))
	{
		t_stack_len = 0;
		return;
	}
	timespec_get(&(t_stack[t_stack_len - 1]), TIME_UTC);
}

struct timespec stop_timer(void)
{
	struct timespec tm = {.tv_sec = 0, .tv_nsec = 0};
	if (t_stack_len <= 0) return tm;

	timespec_get(&tm, TIME_UTC);
	--t_stack_len;
	tm.tv_nsec -= t_stack[t_stack_len].tv_nsec;
	if (tm.tv_nsec < 0)
	{
		tm.tv_nsec += 1000000000L;
		tm.tv_sec -= 1;
	}
	tm.tv_sec -= t_stack[t_stack_len].tv_sec;

	if (!(t_stack = (struct timespec *) realloc(t_stack, sizeof(struct timespec) * t_stack_len))) t_stack_len = 0;
	return tm;
}

void print_elapsed_time(void)
{
	struct timespec tm = stop_timer();
	printf("%2lu.%06lu seconds passed\n", tm.tv_sec, tm.tv_nsec / 1000);
}
