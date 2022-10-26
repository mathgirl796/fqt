#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <zlib.h>
#include <time.h>
#include "fastq_transpose.hpp"
#include "barcode_generate.hpp"

int transpose_main(int argc, char** argv) {
	int buf_size = 1024 * 1024;
	char output_dir[1024];
	size_t len;

	char ch;
	while ((ch = (char)getopt(argc, argv, "b:o:")) != EOF)
	{
		switch (ch)
		{
		case 'b':
			buf_size = atoi(optarg);
			break;
		case 'o':
			strcpy(output_dir, optarg);
			len = strlen(output_dir);
			if (output_dir[len - 1] != '/') strcat(output_dir, "/");
			break;
		default:
			return 1;
		}
	}
	// run
	return fastq_transpose(argv[optind], output_dir, buf_size);
}

int concat_main(int argc, char** argv) {
	int buf_size = 1024 * 1024;
	int lreads;
	char output_dir[1024];
	size_t len;

	char ch;
	while ((ch = (char)getopt(argc, argv, "b:r:o:")) != EOF)
	{
		switch (ch)
		{
		case 'b':
			buf_size = atoi(optarg);
			break;
		case 'r':
			lreads = atoi(optarg);
			break;
		case 'o':
			strcpy(output_dir, optarg);
			len = strlen(output_dir);
			if (output_dir[len - 1] != '/') strcat(output_dir, "/");
			break;
		default:
			return 1;
		}
	}
	int num_sample = argc - optind;
	fprintf(stderr, "concat %d samples...", num_sample);

	return barcode_simulate(argv+optind, output_dir, buf_size, lreads, argc-optind);
}

int viewbc_main(int argc, char **argv) {
	int start = atoi(strtok(argv[1], "-"));
	int end = atoi(strtok(NULL, "-"));
	return barcode_view(argv[2], start, end, 10);
}

void usage() {
	printf("fqt transpose -b buffer_size(default 1M) -o output_dir [fa/fa.gz]\n");
	printf("fqt concat -b buffer_size(default 1M) -o output_dir -r read_length [\"fqt transpose\" output_dirs]\n");
	printf("fqt viewbc 0-20 [\"fqt concat\" output_dir] // Left closed right open interval");
	printf("\n");
}

int main(int argc, char **argv)
{

	if (argc <= 1);
	else if (strcmp(argv[1], "transpose") == 0) return transpose_main(argc - 1, argv + 1);
	else if (strcmp(argv[1], "concat") == 0) return concat_main(argc - 1, argv + 1);
	else if (strcmp(argv[1], "viewbc") == 0) return viewbc_main(argc - 1, argv + 1);

	usage();

	return 0;
}
