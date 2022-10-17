#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <zlib.h>
#include <time.h>
#include "fqt/fqt.h"

int main(int argc, char **argv)
{
	int buf_size = 1024 * 1024;
	char output_dir[1024];

	char ch;
	while ((ch = getopt(argc, argv, "b:o:")) != EOF)
	{
		char *str;
		switch (ch)
		{
		case 'b':
			buf_size = strtol(optarg, &str, 10);
			break;
		case 'o':
			strcpy(output_dir, optarg);
			int len = strlen(output_dir);
			if (output_dir[len - 1] != '/') strcat(output_dir, "/");
			break;
		default:
			return 1;
		}
	}

	// run
	int ret = fqt(argv[optind], output_dir, buf_size);
	
	return 0;
}
