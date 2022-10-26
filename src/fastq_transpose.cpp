#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include "fastq_transpose.hpp"
extern "C" {
    #include "utils.h"
}

int fastq_transpose(char *file_path, char *output_dir, int buf_size)
{
    /* define temp variables */
    gzFile fq_gzfile = gzopen(file_path, "r");
    kseq_t *seq = kseq_init(fq_gzfile);
    int read_len = 0, seq_num = 0, l;
    gzFile *s_ofs; /* output files, totally read_len files */
    gzFile *q_ofs; /* output files, totally read_len files */
    char **s_bufs; /* buffers for each file */
    char **q_bufs; /* buffers for each file */
    char fn[1024];
    int c = 0;
    clock_t t = clock();
    while ((l = kseq_read(seq)) >= 0)
    {
        /* init */
        if (seq_num == 0)
        {
            /* open files */
            s_ofs = (gzFile *)xmalloc(l * sizeof(gzFile));
            q_ofs = (gzFile*)xmalloc(l * sizeof(gzFile));
            
            for (int i = 0; i < l; ++i)
            {
                sprintf(fn, "%s/S_%03d.bin.gz", output_dir, i);
                s_ofs[i] = gzopen(fn, "w");
                sprintf(fn, "%s/Q_%03d.bin.gz", output_dir, i);
                q_ofs[i] = gzopen(fn, "w");
            }
            /* allocate buffers */
            s_bufs = (char**)xmalloc(l * sizeof(char *));
            q_bufs = (char**)xmalloc(l * sizeof(char *));
            for (int i = 0; i < l; ++i)
            {
                s_bufs[i] = (char*)xmalloc(buf_size * sizeof(char));
                q_bufs[i] = (char*)xmalloc(buf_size * sizeof(char));
            }
        }
        seq_num++;
        read_len = l;

        /* really process reads */
        if (c == buf_size)
        {
            for (int i = 0; i < l; ++i)
            {
                err_gzwrite(s_ofs[i], s_bufs[i], c);
                err_gzwrite(q_ofs[i], q_bufs[i], c);
            }
            fprintf(stderr, "processed %d reads, using %.2f sec\r", 
                    seq_num, (float)(clock()-t)/CLOCKS_PER_SEC);
            c = 0;
        }
        for (int i = 0; i < l; ++i)
        {
            s_bufs[i][c] = seq->seq.s[i];
            q_bufs[i][c] = seq->qual.s[i];
        }
        c += 1;
    }
    /* write the last batch */
    if (c > 0)
    {
        for (int i = 0; i < read_len; ++i)
        {
            err_gzwrite(s_ofs[i], s_bufs[i], c);
            err_gzwrite(q_ofs[i], q_bufs[i], c);
        }
    }
    fprintf(stderr, "processed %d reads, using %.2f sec\n", 
            seq_num, (float)(clock()-t)/CLOCKS_PER_SEC);
    kseq_destroy(seq);
    gzclose(fq_gzfile);

    // write meta infomation
    sprintf(fn, "%s/meta.txt", output_dir);
    FILE *f = fopen(fn, "w");
    err_fprintf(f, "%d", seq_num);
    err_fclose(f);

    // free resource
    for (int i = 0; i < read_len; ++i)
    {
        free(s_bufs[i]);
        free(q_bufs[i]);
        gzclose(s_ofs[i]);
        gzclose(q_ofs[i]);
    }
    free(s_bufs);
    free(q_bufs);
    free(s_ofs);
    free(q_ofs);

    return 0;
}
