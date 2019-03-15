#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "arg_wrap.h"

static char help_str[MAX_HELP_STR_SIZE];
void print_help()
{
	fprintf(stdout, strlen(help_str) ? help_str : "Help string not set\n");
}

void set_help_string(char *str) {
	strncpy(help_str, str, MAX_HELP_STR_SIZE - 1);
}

void process_args(const int n_switches, char *opt_switch, char **opt_name, int opt_arg[], void (*opt_func[])(), int argc, char **argv) {
	int count = 0;	// number of mandotary command line options
	int c;
	char opt_str[MAX_OPT_STR_SIZE];
	int i, j;
	for (i = 0, j = 0; i < n_switches && j < MAX_OPT_STR_SIZE - 1; i++) {
		opt_str[j++] = opt_switch[i];
		if (opt_arg[i] == required_argument) {
			opt_str[j++] = ':';
			count++;
		}
	}
	opt_str[j] = '\0';

	struct option long_options[n_switches + 1];
	for (i = 0; i < n_switches; i++) {
		/* These options set a flag. */
		//{"version", no_argument,       &verbose_flag, 1},
		// {"brief",   no_argument,       &verbose_flag, 0},
		/* These options don’t set a flag.
		 *              We distinguish them by their indices. */
		//{"append",  no_argument,       0, 'b'},
		long_options[i].name = opt_name[i];
		long_options[i].has_arg = opt_arg[i];
		long_options[i].flag = 0;
		long_options[i].val = opt_switch[i];
	}
	long_options[n_switches].name = 0;	//Last element needs to be empty
	long_options[n_switches].has_arg = 0;
	long_options[n_switches].flag = 0;
	long_options[n_switches].val = 0;
	while (1) {
		/* getopt_long stores the option index here. */
		int option_index = 0;
		c = getopt_long(argc, argv, opt_str, long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
		switch (c) {
		case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options[option_index].flag != 0)
				break;
			printf("option %s", long_options[option_index].name);
			if (optarg)
				printf(" with arg %s", optarg);
			printf("\n");
			break;
		case '?':
			print_help();
			/* getopt_long already printed an error message. */
			exit(1);
			break;
		default:
			for (i = 0; i < n_switches; i++) {
				if (c == opt_switch[i]) {
					//is_processed = 1;
					if (opt_arg[i] == required_argument) {
						opt_func[i] (optarg);
						count--;
					} else {
						opt_func[i] ();
					}
					break;
				}
			}
		}
	}
	if (count) {
		printf(" Mandatory  command[s] is missing  s, o, m\n");
		print_help();
		exit(1);
	}

	/* Instead of reporting ‘--verbose’
	 *      and ‘--brief’ as they are encountered,
	 *           we report the final status resulting from them. */
		if (verbose_flag)
			puts("verbose flag is set");

	/* Print any remaining command line arguments (not options). */
	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		putchar('\n');
	}
	exit(0);
}

