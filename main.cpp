#include <iostream>
#include <string>
#include "cmdline.hpp"
#include "fqt.hpp"
using namespace std;

int transpose_main(int argc, char** argv) {
    cmdline::parser a;
    a.add<string>("input_file", 'i', "path to the input fastq file", true);
    a.add<string>("output_dir", 'o', "output folder (namely a SAMPLE) to restore output files", false, "./");
    a.add<int>("buf_size", 'b', "memory buffer size for each output file", false, 1048576);
    a.add<int>("read_len", 'l', "read length of the fastq file", true);
    a.parse_check(argc, argv);

    fqt_transpose(
        a.get<string>("input_file").c_str(), 
        (a.get<string>("output_dir")).c_str(),
        a.get<int>("buf_size"),
        a.get<int>("read_len")
    );
    return 0;
}

int concat_main(int argc, char** argv) {
    cmdline::parser a;
    a.add<string>("output_dir", 'o', "output folder to restore output files", false, "./");
    a.add<int>("buf_size", 'b', "memory buffer size for each input SAMPLE and output file", false, 1048576);
    a.add<int>("read_len", 'l', "read length of each SAMPLE", true);
    a.footer("[SAMPLE 1] [SAMPLE 2] ...");
    a.parse_check(argc, argv);

    fqt_concat(
        a.rest(),
        (a.get<string>("output_dir")).c_str(),
        a.get<int>("buf_size"),
        a.get<int>("read_len")
    );
    return 0;
}

int bc_main(int argc, char** argv) {
    cmdline::parser a;
    a.add<string>("input_dir", 'i', "folder which contains barcode.bin.gz file", true);
    a.add<int>("start", 's', "start position", true);
    a.add<int>("end", 'e', "end position", true);
    a.parse_check(argc, argv);

    fqt_bc(
        a.get<string>("input_dir").c_str(), 
        a.get<int>("start"),
        a.get<int>("end")
    );
    return 0;
}

int main(int argc, char **argv)
{

	if (argc <= 1) {
        fprintf(stderr, "%s transpose/concat/bc\n", argv[0]);
    }
	else if (strcmp(argv[1], "transpose") == 0) return transpose_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "concat") == 0) return concat_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "bc") == 0) return bc_main(argc - 1, argv + 1);

	return 0;
}
