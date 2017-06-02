#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "arg_wrap.h"

void fa(){
}
void fb(char *optarg){
}
void fc(char *optarg){
}
void fd(char *optarg){
}

int main (int argc, char **argv)
{
    const int n_switches = 4;
    
    char opt_switch[] = {'v', 's', 'o', 'm', 0};

    char opt_name[][30] ={"version", "size", "name3", "name4", 0};

    int opt_arg[] ={no_argument, required_argument, required_argument, required_argument, 0};

    void (*opt_func[])() = {(void *)&fa, (void *)&fb, (void *)&fc, (void *)&fd, (void *)NULL};

    set_help_string("line a\n"\
                    "line b\n"\
                    "line c\n");
    process_args(n_switches, opt_switch, (char **)opt_name, opt_arg, opt_func, argc, argv);

    exit (0);
}
