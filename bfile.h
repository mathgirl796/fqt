#pragma once
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "utils.h"

#define B_NULL 0
#define B_EOF UINT32_MAX
enum BMODE {B_READ, B_WRITE, BZ_WRITE};

typedef struct bfile_s {
    gzFile      fp;
    enum BMODE  type;
    uint8_t     *buf;
    int         pos;
    int         capacity;
} bfile_s, *bfile;

static inline bfile bopen(const char *filename, const enum BMODE bmode, const int capacity) {
    bfile_s bfs = {Z_NULL, bmode, (uint8_t*)malloc(capacity), 0, capacity}; bfile bf = (bfile)malloc(sizeof(bfile_s)); *bf = bfs;
    if      (bf->type == B_READ)     bf->fp = xzopen(filename, "rb");
    else if (bf->type == BZ_WRITE)    bf->fp = xzopen(filename, "wb");
    else if (bf->type == B_WRITE)    bf->fp = xzopen(filename, "wbT");
    return bf;
}

static inline uint32_t bgetc(bfile bf) {
    if (bf->pos == 0) { bf->pos = err_gzread(bf->fp, bf->buf, bf->capacity); memmove(bf->capacity - bf->pos + bf->buf, bf->buf, bf->pos); }
    if (bf->pos <= 0) { return B_EOF; }
    return (uint32_t)bf->buf[bf->capacity - bf->pos--];
}

static inline uint32_t bputc(uint8_t ch, bfile bf) {
    if (bf->pos == bf->capacity) { err_gzwrite(bf->fp, bf->buf, bf->pos); bf->pos = 0; }
    return (uint32_t)(bf->buf[bf->pos++] = ch);
}

static inline void bprintf(bfile bf, const char *fmt, ...) {
    uint8_t buf[1024];
    va_list ap; va_start(ap, fmt);
    vsprintf((char*)buf, fmt, ap);
    for (int i = 0; i < strlen((char*)buf); ++i) {
        bputc(buf[i], bf);
    }
}

static inline int bclose(bfile bf) {
    if (bf->type != B_READ && bf->pos > 0) { err_gzwrite(bf->fp, bf->buf, bf->pos); }
    int ret = err_gzclose(bf->fp);    free(bf->buf);  free(bf);     return ret;
}
