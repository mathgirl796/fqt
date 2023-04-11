#pragma once

#include <fstream>
#include <zlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include "json.hpp"
#include "bfile.h"
#include "utils.h"
#include "choicer.hpp"
using json = nlohmann::json;

static inline void fqt_transpose(const char *file_path, const char *output_dir, int buf_size, int read_len)
{
    gzFile fp = xzopen(file_path, "r");
    kseq_t *seq = kseq_init(fp);
    bfile bout_seq[read_len];
    bfile bout_qual[read_len];
    char fn[1024];
    int len, read_num = 0;

    xmkdir(output_dir);
    for (int i = 0; i < read_len; ++i)
    {
        sprintf(fn, "%s/S_%03d.bin.gz", output_dir, i);
        bout_seq[i] = bopen(fn, BZ_WRITE, buf_size);
        sprintf(fn, "%s/Q_%03d.bin.gz", output_dir, i);
        bout_qual[i] = bopen(fn, BZ_WRITE, buf_size);
    }

    clock_t t = clock();
    while ((len = kseq_read(seq)) >= 0)
    {
        read_num += 1;
        for (int i = 0; i < read_len; ++i)
        {
            bputc(seq->seq.s[i], bout_seq[i]);
            bputc(seq->qual.s[i], bout_qual[i]);
        }
        if (read_num % one_GigaByte == 0)
        {
            fprintf(stderr, "Processed %d reads, using %.2f sec.\n", read_num, (float)(clock() - t) / CLOCKS_PER_SEC);
        }
    }
    for (int i = 0; i < read_len; ++i)
    {
        bclose(bout_seq[i]);
        bclose(bout_qual[i]);
    }
    fprintf(stderr, "Finished, processed %d reads, using %.2f sec.\n", read_num, (float)(clock() - t) / CLOCKS_PER_SEC);

    sprintf(fn, "%s/meta.json", output_dir);
    json meta = {{"read_num", read_num}};
    std::ofstream ofile(fn);
    ofile << meta;
    ofile.close();
    fprintf(stderr, "Generated %s.\n", fn);

    sprintf(fn, "%s/barcode.bin.gz", output_dir);
    bfile barcode_file = bopen(fn, BZ_WRITE, buf_size);
    for (int i = 0; i < read_len * 2; ++i)
    {
        bputc(0, barcode_file);
    }
    bclose(barcode_file);
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
        json meta;
        sprintf(fn, "%s/%s", input_dirs[i].c_str(), "meta.json");
        FILE* file = xopen(fn, "r");
        meta = json::parse(file);
        fclose(file);
        nreads[i] = meta["read_num"];
        read_num += nreads[i];
    }
    fprintf(stderr, "Totally %d reads.\n", read_num);

    xmkdir(output_dir);
    bfile bin_seq[read_len][nsample];
    bfile bin_qual[read_len][nsample];
    bfile bout_seq[read_len];
    bfile bout_qual[read_len];
    bfile bout_bc;
    for (int i = 0; i < read_len; ++i)
    {
        for (int j = 0; j < nsample; ++j)
        {
            sprintf(fn, "%s/S_%03d.bin.gz", input_dirs[j].c_str(), i);
            bin_seq[i][j] = bopen(fn, B_READ, buf_size);
            sprintf(fn, "%s/Q_%03d.bin.gz", input_dirs[j].c_str(), i);
            bin_qual[i][j] = bopen(fn, B_READ, buf_size);
        }
        sprintf(fn, "%s/S_%03d.bin.gz", output_dir, i);
        bout_seq[i] = bopen(fn, BZ_WRITE, buf_size);
        sprintf(fn, "%s/Q_%03d.bin.gz", output_dir, i);
        bout_qual[i] = bopen(fn, BZ_WRITE, buf_size);
    }
    sprintf(fn, "%s/%s", output_dir, "barcode.bin.gz");
    bout_bc = bopen(fn, BZ_WRITE, buf_size);
    Choicer choicer(nsample, nreads);
    uint16_t choice;
    clock_t t = clock();
    int read_num_counter = 0;
    while ((choice = choicer.next()) != (uint16_t) -1)
    {
        read_num_counter += 1; //fprintf(stderr, "%d\t%d\t%d\t%d\t%d\n", read_num_counter, choicer.nreads_status[0], choicer.nreads_status[1], choicer.v.size(), choice);
        for (int i = 0; i < read_len; ++i)
        {
            bputc(bgetc(bin_seq[i][choice]), bout_seq[i]);
            bputc(bgetc(bin_qual[i][choice]), bout_qual[i]);
        }
        bputc(((uint8_t*)(&choice))[0], bout_bc);
        bputc(((uint8_t*)(&choice))[1], bout_bc);
        if (read_num % one_GigaByte == 0)
        {
            fprintf(stderr, "Processed %d reads, using %.2f sec.\n", read_num_counter, (float)(clock() - t) / CLOCKS_PER_SEC);
        }
    }

    for (int i = 0; i < read_len; ++i)
    {
        for (int j = 0; j < nsample; ++j)
        {
            bclose(bin_seq[i][j]);
            bclose(bin_qual[i][j]);
        }
        bclose(bout_seq[i]);
        bclose(bout_qual[i]);
    }
    bclose(bout_bc);
    fprintf(stderr, "Finished, processed %d reads, using %.2f sec.\n", read_num_counter, (float)(clock() - t) / CLOCKS_PER_SEC);

    sprintf(fn, "%s/meta.json", output_dir);
    json meta = {{"read_num", read_num}};
    std::ofstream ofile(fn);
    ofile << meta;
    ofile.close();
    fprintf(stderr, "Generated %s.\n", fn);
}

static inline void fqt_bc(const char *input_dir, int start, int end)
{
    char fn[1024];
    json meta;
    int read_num;
    gzFile gf;
    uint16_t *barcodes;

    sprintf(fn, "%s/meta.json", input_dir);
    FILE* file = xopen(fn, "r");
    meta = json::parse(file);
    fclose(file);

    read_num = meta["read_num"];
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