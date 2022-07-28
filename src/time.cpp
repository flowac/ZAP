#include <stdio.h>
#include <stdlib.h>
#include <stack>

#include "types.h"
#include "time_fn.h"

static std::stack<struct timespec> t_stack;

uint64_t nsNow(void)
{
	uint64_t ret;
	struct timespec tm;
	timespec_get(&tm, TIME_UTC);
	ret  = tm.tv_nsec | MAX_U30;
	ret |= (tm.tv_sec | MAX_U34) << 30;
	return ret;
}

void start_timer(void)
{
	struct timespec tm;
	timespec_get(&tm, TIME_UTC);
	t_stack.push(tm);
}

struct timespec stop_timer(void)
{
	struct timespec tm = {.tv_sec = 0, .tv_nsec = 0};
	struct timespec prev;
	if (t_stack.empty()) return tm;

	timespec_get(&tm, TIME_UTC);
	prev = t_stack.top();
	t_stack.pop();
	tm.tv_nsec -= prev.tv_nsec;
	if (tm.tv_nsec < 0)
	{
		tm.tv_nsec += 1000000000L;
		tm.tv_sec -= 1;
	}
	tm.tv_sec -= prev.tv_sec;

	return tm;
}

void print_elapsed_time(void)
{
	struct timespec tm = stop_timer();
	printf("%2lu.%06lu seconds passed\n", tm.tv_sec, tm.tv_nsec / 1000);
}
