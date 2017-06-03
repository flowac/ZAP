#include <getopt.h>

/* Flag set by ‘--verbose’. */
static int verbose_flag;
#define MAX_FILE_PATH 255
#define MAX_OPT_STR_SIZE 100
#define MAX_HELP_STR_SIZE 400

/*  @params: help string for the tool
*/
void set_help_string(char *str);

/*  @params: total number of switches, switch shorthand, switch full name,
             wheter or not option is required, function pointers for processing the commands,
             argc and argv from main function
*/
void process_args(  const int n_switches,   //  total number of switches
                    char *opt_switch,       //  switch shorthand
                    char **opt_name,        //  switch full name    
                    int opt_arg[],          //  is option required (0:no, 1:yes, 2:optional)
                    void (*opt_func[])(),   //  fucntion pointer to handle switches
                    int argc,               //  argc from main
                    char **argv);           //  argv from main  
                    