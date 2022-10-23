#ifndef __FQT_H
#define __FQT_H

/**
 * @param file_path fastq file path
 * @param output_dir output files dir
 * @param buf_size buffer size for each base in a read
 * 
 * @return 0
 */
int fastq_transpose(char *file_path, char *output_dir, int buf_size);

#endif