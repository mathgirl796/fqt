#include <stdio.h>
#include <zlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include "fqt.h"
#include "kseq.h"
KSEQ_INIT(gzFile, gzread)

int mkdirs(const char *muldir)
{
    int i, len;
    char str[512];
    strncpy(str, muldir, 512);
    len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (str[i] == '/' && i != 0)
        {
            str[i] = '\0';
            if (access(str, 0) != 0)
            {
                if ((mkdir(str, 0777)) == -1)
                    return FQT_ERR_MKDIR;
            }
            str[i] = '/';
        }
    }
    if (len > 0 && access(str, 0) != 0)
    {
        mkdir(str, 0777);
    }
    return FQT_OK;
}

int fqt(char *file_path, char *output_dir, int buf_size)
{
    fprintf(stderr, 
        "fqt file_path: %s\n    output_dir: %s\n    buf_size: %d\n",
        file_path, output_dir, buf_size);

    /* make output dir */
    if (mkdirs(output_dir) != FQT_OK)
    {
        return FQT_ERR_MKDIR;
    }

    /* define temp variables */
    gzFile fq_gzfile;
    if ((fq_gzfile = gzopen(file_path, "r")) == Z_NULL)
        return FQT_ERR_OPEN_FILE;
    kseq_t *seq = kseq_init(fq_gzfile);
    int read_len = 0, seq_num = 0, l;
    gzFile *s_ofs; /* output files, totally read_len files */
    gzFile *q_ofs; /* output files, totally read_len files */
    char **s_bufs; /* buffers for each file */
    char **q_bufs; /* buffers for each file */
    int c = 0;
    clock_t t = clock();
    while ((l = kseq_read(seq)) >= 0)
    {
        /* init */
        if (seq_num == 0)
        {
            /* open files */
            s_ofs = malloc(l * sizeof(gzFile));
            q_ofs = malloc(l * sizeof(gzFile));
            char fn[1024];
            for (int i = 0; i < l; ++i)
            {
                sprintf(fn, "%s/S_%03d.bin.gz", output_dir, i);
                if ((s_ofs[i] = gzopen(fn, "w")) == Z_NULL)
                    return FQT_ERR_OPEN_FILE;
                sprintf(fn, "%s/Q_%03d.bin.gz", output_dir, i);
                if ((q_ofs[i] = gzopen(fn, "w")) == Z_NULL)
                    return FQT_ERR_OPEN_FILE;
            }
            /* allocate buffers */
            s_bufs = malloc(l * sizeof(char *));
            q_bufs = malloc(l * sizeof(char *));
            for (int i = 0; i < l; ++i)
            {
                s_bufs[i] = malloc(buf_size * sizeof(char));
                if (s_bufs[i] == NULL)
                    return FQT_ERR_ALLOC;
                q_bufs[i] = malloc(buf_size * sizeof(char));
                if (q_bufs[i] == NULL)
                    return FQT_ERR_ALLOC;
            }
        }
        /* deal with read length unconsistency */
        else if (l != read_len)
        {
            return FQT_ERR_CONSISTENCY;
        }
        seq_num++;
        read_len = l;

        /* really process reads */
        if (c == buf_size)
        {
            for (int i = 0; i < l; ++i)
            {
                int s_errno = gzwrite(s_ofs[i], s_bufs[i], c);
                int q_errno = gzwrite(q_ofs[i], q_bufs[i], c);
                if (s_errno <= 0 || q_errno <= 0)
                    return FQT_ERR_WRITE;
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
            int s_errno = gzwrite(s_ofs[i], s_bufs[i], c);
            int q_errno = gzwrite(q_ofs[i], q_bufs[i], c);
            if (s_errno <= 0 || q_errno <= 0)
                return FQT_ERR_WRITE;
        }
    }
    fprintf(stderr, "processed %d reads, using %.2f sec\n", 
            seq_num, (float)(clock()-t)/CLOCKS_PER_SEC);
    kseq_destroy(seq);
    gzclose(fq_gzfile);
    for (int i = 0; i < read_len; ++i)
    {
        free(s_bufs[i]);
        free(q_bufs[i]);
        int s_errno = gzclose(s_ofs[i]);
        int q_errno = gzclose(q_ofs[i]);
        if (s_errno != Z_OK || q_errno != Z_OK)
            return FQT_ERR_CLOSE_FILE;
    }
    free(s_bufs);
    free(q_bufs);
    free(s_ofs);
    free(q_ofs);

    return FQT_OK;
}
