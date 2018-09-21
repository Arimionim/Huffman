#include "lib/huffman.h"
#include <iostream>
#include <fstream>
#include <iomanip>


int main(int argc, char *argv[]) {
    enum huffman_mode_t {
        COMPRESS, DECOMPRESS
    };

    if (argc != 4) {
        std::cerr << "Error: wrong args";
        return 1;
    }

    std::string mode_arg = argv[1];
    huffman_mode_t mode;

    if (mode_arg == "-c" || mode_arg == "--compress") {
        mode = COMPRESS;
    } else if (mode_arg == "-d" || mode_arg == "--decompress") {
        mode = DECOMPRESS;
    } else {
        std::cerr << "Unknown option: " << mode_arg << std::endl;
        return 1;
    }

    std::string input_path = argv[2];
    std::string output_path = argv[3];

    if (input_path == output_path) {
        std::cerr << "Please specify different input and output files" << std::endl;
        return 2;
    }

    try {
        std::ifstream input_file(input_path, std::ios::binary);
        if (input_file.fail())
            throw std::runtime_error("cannot open " + input_path);

        std::ofstream output_file(output_path, std::ios::binary);
        if (output_file.fail())
            throw std::runtime_error("cannot open " + output_path);

        if (mode == COMPRESS) {
            std::cerr << "Starting compressing.." << std::endl;
            huffman::compress(input_file, output_file);
        } else {
            std::cerr << "Starting decompressing.." << std::endl;
            huffman::decompress(input_file, output_file);
        }

        std::cerr << "Finished" << std::endl;

        return 0;

    } catch (std::exception &e) {
        std::cerr << "ERROR" << e.what() << std::endl;
        return 2;
    }
}