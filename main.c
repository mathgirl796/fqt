#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <zlib.h>
#include <time.h>
#include "kseq.h"
#include "fqt.h"
KSEQ_INIT(gzFile, gzread);

#define MAX_FN_LEN 1024

void usage()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:  fqt [options] <foo.fastq>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "    -b, buffer size which will be distributed to each\n"
					"        base in a read, default 150M\n");
	fprintf(stderr, "    -o, output dir, default current dir\n");
	fprintf(stderr, "    -s, skip read length consistency check, "
					"        default UNSET\n");
}

int main(int argc, char **argv)
{
	int buf_size = 150 * 1024 * 1024;
	char *file_path = 0;
	char output_dir[MAX_FN_LEN];
	int skip_read_len_check = false;
	clock_t t; /* temp time */

	if (getcwd(output_dir, MAX_FN_LEN) == NULL)
	{
		fprintf(stderr, "ERROR get default output dir\n");
		exit(1);
	}

	char ch;
	while ((ch = getopt(argc, argv, "sb:o:")) != EOF)
	{
		char *str;
		switch (ch)
		{
		case 'b':
			buf_size = strtol(optarg, &str, 10);
			if (*str == 'G' || *str == 'g')
				buf_size *= 1024 * 1024 * 1024;
			else if (*str == 'M' || *str == 'm')
				buf_size *= 1024 * 1024;
			else if (*str == 'K' || *str == 'k')
				buf_size *= 1024;
			break;
		case 'o':
			strcpy(output_dir, optarg);
			int len = strlen(output_dir);
			if (output_dir[len - 1] == '/')
				output_dir[len - 1] = '\0';
			break;
		case 's':
			skip_read_len_check = true;
			break;
		default:
			usage();
			return 1;
		}
	}

	/* process file_path */
	t = clock();
	if (optind == argc)
	{
		usage();
		return 1;
	}
	file_path = argv[optind];
	gzFile fp;
	if ((fp = gzopen(file_path, "r")) == Z_NULL)
	{
		fprintf(stderr, "ERROR open file\n");
		exit(1);
	};
	fprintf(stderr, "check read length consistency...\n");
	kseq_t *seq = kseq_init(fp);
	int read_len = 0, seq_num = 0, l;
	clock_t last = clock();
	while ((l = kseq_read(seq)) >= 0)
	{
		if (seq_num >= 1 && l != read_len)
		{
			fprintf(stderr, "ERROR read num not consistent\n");
			exit(1);
		}
		seq_num++;
		read_len = l;
		float inv = (clock() - last) / CLOCKS_PER_SEC;
		if (skip_read_len_check) break;
		if (inv >= 1)
		{
			fprintf(stderr, "checked %d reads, using %.2f sec\r",
					seq_num, (float)(clock() - t) / CLOCKS_PER_SEC);
			last = clock();
		}
	}
	fprintf(stderr, "finish check, read_num: %d, consumed time: %.2f sec\n",
			seq_num, (float)(clock() - t) / CLOCKS_PER_SEC);
	kseq_destroy(seq);
	gzclose(fp);

	/* process buf_size and run */
	if (seq_num < 1)
	{
		fprintf(stderr, "ERROR read fastq file\n");
		exit(1);
	}
	t = clock();
	int ret = fqt(file_path, output_dir, buf_size / read_len);

	switch (ret)
	{
	case FQT_ERR_CONSISTENCY:
		fprintf(stderr, "ERROR read num not consistent\n");
		exit(1);
		break;
	case FQT_ERR_OPEN_FILE:
		fprintf(stderr, "ERROR open input file\n");
		exit(1);
		break;
	case FQT_ERR_MKDIR:
		fprintf(stderr, "ERROR make output dir\n");
		exit(1);
		break;
	case FQT_ERR_ALLOC:
		fprintf(stderr, "ERROR alloc buffer\n");
		exit(1);
		break;
	case FQT_ERR_WRITE:
		fprintf(stderr, "ERROR write data to disk\n");
		exit(1);
		break;
	case FQT_ERR_CLOSE_FILE:
		fprintf(stderr, "ERROR close file\n");
		exit(1);
		break;
	case FQT_OK:
		fprintf(stderr, "%.2f sec\n", (float)(clock() - t) / CLOCKS_PER_SEC);
		break;
	default:
		fprintf(stderr, "ERROR unknown\n");
		exit(1);
		break;
	}

	return 0;
}
