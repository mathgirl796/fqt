#include "barcode_generate.hpp"
#include <ctime>
extern "C" {
	#include "../clib/utils.h"
}

// for each sample's S/Q and the final output sample we allocate a buf_size buffer
int barcode_simulate(char** input_dirs, char* output_dir, int buf_size, int lread, int nsample)
{
	char fn[1024];
	int nreads[nsample];
	std::vector<uint16_t> buf_bc; //data to be wrote to barcode.bin.gz
	std::vector<uint8_t> sbuf_w;
	std::vector<uint8_t> qbuf_w;
	uint8_t* sbuf_r[nsample];
	uint8_t* qbuf_r[nsample];
	int size[nsample];
	int idx[nsample];
	int choice;
	gzFile bf;
	gzFile sf[nsample];
	gzFile qf[nsample];
	gzFile sf_out, qf_out;
	clock_t t = clock();

	// get nreads
	int seq_num = 0;
	for (int i = 0; i < nsample; ++i) {
		sprintf(fn, "%s/%s", input_dirs[i], "meta.txt");
		FILE* f = fopen(fn, "r");
		fscanf(f, "%d", nreads + i);
		seq_num += nreads[i];
		err_fclose(f);
	}

	// get choicer
	Choicer choicer(nsample, nreads);

	// write meta infomation
	sprintf(fn, "%s/meta.txt", output_dir);
	FILE* f = fopen(fn, "w");
	err_fprintf(f, "%d", seq_num);
	err_fclose(f);

	// generate barcode.bin.gz
	sprintf(fn, "%s/%s", output_dir, "barcode.bin.gz");
	bf = gzopen(fn, "wb");
	choicer.reset();
	while ((choice = choicer.next()) >= 0) {
		buf_bc.emplace_back(choice);
		if (buf_bc.size() * sizeof(uint16_t) >= buf_size) {
			err_gzwrite(bf, buf_bc.data(), buf_bc.size() * sizeof(uint16_t));
			buf_bc.clear();
		}
	}
	if (buf_bc.size() > 0) err_gzwrite(bf, buf_bc.data(), buf_bc.size() * sizeof(uint16_t));
	err_gzclose(bf);
	fprintf(stderr, "generated barcode.bin.gz, using %.2f sec\n", (float)(clock() - t) / CLOCKS_PER_SEC);
	t = clock();

	// generate concat files
	for (int i = 0; i < nsample; ++i) {
		sbuf_r[i] = (uint8_t*)xmalloc(buf_size);
		qbuf_r[i] = (uint8_t*)xmalloc(buf_size);
	}
	for (int i = 0; i < lread; ++i) {
		sprintf(fn, "%s/S_%03d.bin.gz", output_dir, i);
		sf_out = gzopen(fn, "wb");
		sprintf(fn, "%s/Q_%03d.bin.gz", output_dir, i);
		qf_out = gzopen(fn, "wb");

		for (int j = 0; j < nsample; ++j) {
			sprintf(fn, "%s/S_%03d.bin.gz", input_dirs[j], i);
			sf[j] = gzopen(fn, "rb");
			sprintf(fn, "%s/Q_%03d.bin.gz", input_dirs[j], i);
			qf[j] = gzopen(fn, "rb");

			size[j] = 0;
			idx[j] = 0;
		}
		choicer.reset();
		while ((choice = choicer.next()) >= 0) {
			if (idx[choice] == size[choice]) { // there must be new block to read
				size[choice] = err_gzread(sf[choice], sbuf_r[choice], buf_size);
				size[choice] = err_gzread(qf[choice], qbuf_r[choice], buf_size);
				idx[choice] = 0;
			}
			if (size[choice] <= 0) { // if no new block, that must be error
				fprintf(stderr, "fuck you\n");
				exit(1);
			}
			sbuf_w.emplace_back(sbuf_r[choice][idx[choice]]);
			qbuf_w.emplace_back(qbuf_r[choice][idx[choice]]);
			if (sbuf_w.size() == buf_size) {
				err_gzwrite(sf_out, sbuf_w.data(), sbuf_w.size()); sbuf_w.clear();
				err_gzwrite(qf_out, qbuf_w.data(), qbuf_w.size()); qbuf_w.clear();
			}
			idx[choice] += 1;
		}

		if (sbuf_w.size() > 0) {
			err_gzwrite(sf_out, sbuf_w.data(), sbuf_w.size()); sbuf_w.clear();
			err_gzwrite(qf_out, qbuf_w.data(), qbuf_w.size()); qbuf_w.clear();
		}
		err_gzclose(sf_out);
		err_gzclose(qf_out);
		for (int j = 0; j < nsample; ++j) {
			err_gzclose(sf[j]);
			err_gzclose(qf[j]);
		}
		fprintf(stderr, "processed cycle %d, using %.2f sec\r", i, (float)(clock() - t) / CLOCKS_PER_SEC);
	}
	fprintf(stderr, "\n");
	for (int i = 0; i < nsample; ++i) {
		free(sbuf_r[i]);
		free(qbuf_r[i]);
	}
}