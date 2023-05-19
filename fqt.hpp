#pragma once

#include <fstream>
#include <zlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include "utils.h"
#include "choicer.hpp"
#define BUF_SIZE 1000000

static inline void fqt_transpose(const char *file_path, const char *output_dir, int buf_size, int read_len)
{
    gzFile fp = xzopen(file_path, "r");
    kseq_t *seq = kseq_init(fp);
    gzFile bout_seq[read_len];
    gzFile bout_qual[read_len];
    char fn[1024];
    int len, read_num = 0;

    xmkdir(output_dir);
    for (int i = 0; i < read_len; ++i)
    {
        sprintf(fn, "%s/S_%03d.bin.gz", output_dir, i);
        bout_seq[i] = xzopen(fn, "w"); gzbuffer(bout_seq[i], BUF_SIZE);
        sprintf(fn, "%s/Q_%03d.bin.gz", output_dir, i);
        bout_qual[i] = xzopen(fn, "w"); gzbuffer(bout_qual[i], BUF_SIZE);
    }

    clock_t t = clock();
    while ((len = kseq_read(seq)) >= 0)
    {
        read_num += 1;
        for (int i = 0; i < read_len; ++i)
        {
            gzputc(bout_seq[i], seq->seq.s[i]);
            gzputc(bout_qual[i], seq->qual.s[i]);
        }
        if (read_num % 100000 == 0)
        {
            fprintf(stderr, "Processed %d reads, using %.2f sec.\n", read_num, (float)(clock() - t) / CLOCKS_PER_SEC);
        }
    }
    for (int i = 0; i < read_len; ++i)
    {
        err_gzclose(bout_seq[i]);
        err_gzclose(bout_qual[i]);
    }
    fprintf(stderr, "Finished, processed %d reads, using %.2f sec.\n", read_num, (float)(clock() - t) / CLOCKS_PER_SEC);

    sprintf(fn, "%s/meta.txt", output_dir);
    FILE* meta = xopen(fn, "w");
    err_fprintf(meta, "%d", read_num);
    err_fclose(meta);
    fprintf(stderr, "Generated %s.\n", fn);

    sprintf(fn, "%s/barcode.bin.gz", output_dir);
    gzFile barcode_file = xzopen(fn, "w"); gzbuffer(barcode_file, BUF_SIZE);
    for (int i = 0; i < read_len * 2; ++i)
    {
        gzputc(barcode_file, 0);
    }
    err_gzclose(barcode_file);
    fprintf(stderr, "Generated %s filled with zero.\n", fn);
}

static inline void fqt_concat(std::vector<std::string> input_dirs, const char *output_dir, int buf_size, int read_len)
{
    char fn[1024];
    int nsample = input_dirs.size();
    int nreads[nsample];
    int read_num = 0;

    if (nsample <= 0) {
        fprintf(stderr, "No sample input.\n");
        exit(1);
    }
    for (int i = 0; i < nsample; ++i)
    {
        sprintf(fn, "%s/%s", input_dirs[i].c_str(), "meta.txt");
        FILE* meta = xopen(fn, "r");
        fscanf(meta, "%d", nreads + i);
        fclose(meta);
        read_num += nreads[i];
    }
    fprintf(stderr, "Totally %d reads.\n", read_num);

    xmkdir(output_dir);
    gzFile bin_seq[read_len][nsample];
    gzFile bin_qual[read_len][nsample];
    gzFile bout_seq[read_len];
    gzFile bout_qual[read_len];
    gzFile bout_bc;
    for (int i = 0; i < read_len; ++i)
    {
        for (int j = 0; j < nsample; ++j)
        {
            sprintf(fn, "%s/S_%03d.bin.gz", input_dirs[j].c_str(), i);
            bin_seq[i][j] = xzopen(fn, "r"); gzbuffer(bin_seq[i][j], BUF_SIZE);
            sprintf(fn, "%s/Q_%03d.bin.gz", input_dirs[j].c_str(), i);
            bin_qual[i][j] = xzopen(fn, "r"); gzbuffer(bin_qual[i][j], BUF_SIZE);
        }
        sprintf(fn, "%s/S_%03d.bin.gz", output_dir, i);
        bout_seq[i] = xzopen(fn, "w"); gzbuffer(bout_seq[i], BUF_SIZE);
        sprintf(fn, "%s/Q_%03d.bin.gz", output_dir, i);
        bout_qual[i] = xzopen(fn, "w"); gzbuffer(bout_qual[i], BUF_SIZE);
    }
    sprintf(fn, "%s/%s", output_dir, "barcode.bin.gz");
    bout_bc = xzopen(fn, "w"); gzbuffer(bout_bc, BUF_SIZE);
    Choicer choicer(nsample, nreads);
    uint16_t choice;
    clock_t t = clock();
    int read_num_counter = 0;
    while ((choice = choicer.next()) != (uint16_t) -1)
    {
        read_num_counter += 1; //fprintf(stderr, "%d\t%d\t%d\t%d\t%d\n", read_num_counter, choicer.nreads_status[0], choicer.nreads_status[1], choicer.v.size(), choice);
        for (int i = 0; i < read_len; ++i)
        {
            gzputc(bout_seq[i], gzgetc(bin_seq[i][choice]));
            gzputc(bout_qual[i], gzgetc(bin_qual[i][choice]));
        }
        err_gzwrite(bout_bc, &choice, sizeof(uint16_t));
        if (read_num % one_GigaByte == 0)
        {
            fprintf(stderr, "Processed %d reads, using %.2f sec.\n", read_num_counter, (float)(clock() - t) / CLOCKS_PER_SEC);
        }
    }

    for (int i = 0; i < read_len; ++i)
    {
        for (int j = 0; j < nsample; ++j)
        {
            err_gzclose(bin_seq[i][j]);
            err_gzclose(bin_qual[i][j]);
        }
        err_gzclose(bout_seq[i]);
        err_gzclose(bout_qual[i]);
    }
    err_gzclose(bout_bc);
    fprintf(stderr, "Finished, processed %d reads, using %.2f sec.\n", read_num_counter, (float)(clock() - t) / CLOCKS_PER_SEC);

    sprintf(fn, "%s/meta.txt", output_dir);
    FILE* meta = xopen(fn, "w");
    err_fprintf(meta, "%d", read_num);
    err_fclose(meta);
    fprintf(stderr, "Generated %s.\n", fn);
}

static inline void fqt_bc(const char *input_dir, int start, int end)
{
    char fn[1024];
    int read_num;
    gzFile gf;
    uint16_t *barcodes;

    sprintf(fn, "%s/meta.txt", input_dir);
    FILE* file = xopen(fn, "r");
    fscanf(file, "%d", &read_num);
    fclose(file);

    start = start > 0 ? start : 0;
    end = end < read_num ? end : read_num;
    barcodes = (uint16_t *)malloc(sizeof(uint16_t) * (end - start));

    sprintf(fn, "%s/barcode.bin.gz", input_dir);
    gf = gzopen(fn, "rb");
    gzseek(gf, start * sizeof(uint16_t), SEEK_SET);
    err_gzread(gf, barcodes, (end - start) * sizeof(uint16_t));
    err_gzclose(gf);

    for (int i = 0; i < (end - start); ++i)
    {
        fprintf(stdout, "%d\t", barcodes[i]);
    }
    fprintf(stdout, "\n");
}