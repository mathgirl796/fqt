#pragma once
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <errno.h>
#include "utils.h"

#define B_NULL 0
enum BMODE {B_READ, B_WRITE};

typedef struct bfile_s {
    gzFile      fp;
    enum BMODE  type;
    char        *buf;
    int         pos;
    int         capacity;
} bfile_s, *bfile;

static inline bfile bopen(const char *filename, const enum BMODE bmode, const int capacity) {
    bfile_s bfs = {Z_NULL, bmode, (char*)malloc(capacity), 0, capacity}; bfile bf = (bfile)malloc(sizeof(bfile_s)); *bf = bfs;
    if      (bf->type == B_READ)     bf->fp = xzopen(filename, "rb");
    else if (bf->type == B_WRITE)    bf->fp = xzopen(filename, "wb");
    return bf;
}

static inline int bgetc(bfile bf) {
    if (bf->type == B_WRITE) return -1;
    if (bf->pos == 0) { bf->pos = err_gzread(bf->fp, bf->buf, bf->capacity); memcpy(bf->capacity - bf->pos + bf->buf, bf->buf, bf->pos); }
    if (bf->pos <= 0) { return bf->pos; }
    return bf->buf[bf->capacity - bf->pos--];
}

static inline int bputc(int ch, bfile bf) {
    if (bf->type == B_READ) return -1;
    if (bf->pos == bf->capacity) { err_gzwrite(bf->fp, bf->buf, bf->pos); bf->pos = 0; }
    bf->buf[bf->pos++] = ch; return ch;
}

static inline int bclose(bfile bf) {
    if (bf->type == B_WRITE && bf->pos > 0) { err_gzwrite(bf->fp, bf->buf, bf->pos); }
    int ret = err_gzclose(bf->fp);    free(bf->buf);  free(bf);     return ret;
}
