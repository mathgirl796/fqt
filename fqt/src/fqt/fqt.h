#ifndef __FQT_H
#define __FQT_H

#define FQT_OK 0
#define FQT_ERR_CONSISTENCY 1
#define FQT_ERR_OPEN_FILE 2
#define FQT_ERR_MKDIR 3
#define FQT_ERR_ALLOC 4
#define FQT_ERR_WRITE 5
#define FQT_ERR_CLOSE_FILE 6

/**
 * @param file_path fastq file path
 * @param output_dir output files dir
 * @param buf_size buffer size for each base in a read
 * 
 * @return 0
 */
int fqt(char *file_path, char *output_dir, int buf_size);

#endif