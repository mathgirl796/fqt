# fqt

## Compile

gcc main.cpp utils.c -lz -lstdc++ -o fqt

## Usage

fqt transpose: transpose fastq format to per cycle format

    ./fqt transpose
    usage: transpose --input_file=string --read_len=int [options] ... 
    options:
    -i, --input_file    path to the input fastq file (string)
    -o, --output_dir    output folder (namely a SAMPLE) to restore output files (string [=./])
    -b, --buf_size      memory buffer size for each output file (int [=1048576])
    -l, --read_len      read length of the fastq file (int)
    -?, --help          print this message


fqt concat: randomly combine multiple cycle format SAMPLE to one cycle format SAMPLE, generate responding barcode file

    ./fqt concat
    usage: concat --read_len=int [options] ... [SAMPLE 1] [SAMPLE 2] ...
    options:
    -o, --output_dir    output folder to restore output files (string [=./])
    -b, --buf_size      memory buffer size for each input SAMPLE and output file (int [=1048576])
    -l, --read_len      read length of each SAMPLE (int)
    -?, --help          print this message

fqt bc: view uint16_t format barcode file in console

    ./fqt bc
    usage: bc --input_dir=string --start=int --end=int [options] ... 
    options:
    -i, --input_dir    folder which contains barcode.bin.gz file (string)
    -s, --start        start position (int)
    -e, --end          end position (int)
    -?, --help         print this message