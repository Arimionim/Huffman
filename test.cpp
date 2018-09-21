#include <iostream>
#include <fstream>
#include <ctime>
#include "lib/huffman.h"

void gen(unsigned int size, std::string const &dst) {
    srand(static_cast<unsigned int>(time(NULL)));
    std::ofstream fout(dst, std::ios::out | std::ios::binary);
    for (int i = 0; i < size; i++) {
        fout << char(rand() % 256);
    }
}

bool check(std::string const & first, std::string const & second){
    std::ifstream a(first, std::ios::binary);
    std::ifstream b(second, std::ios::binary);
    char ta, tb;
    while(a >> ta){
        if (!(b >> tb)){
            std::cout << "false\n";
            return 0;
        }
        if (ta != tb){
            std::cout << "false\n";
            return 0;
        }
    }
    if (b >> tb){
        std::cout << "false\n";
        return 0;
    }
    std::cout << "true\n";
    return 0;
}

void compress(std::string const & first, std::string const & second){
    std::ifstream in(first, std::ios::binary);
    std::ofstream out(second, std::ios::binary);
    huffman::compress(in, out);
}

void decompress(std::string const & first, std::string const & second){
    std::ifstream in(first, std::ios::binary);
    std::ofstream out(second, std::ios::binary);
    huffman::decompress(in, out);
}

int main(int argc, char *argv[]){
    unsigned int cnt[argc - 1];
    for (size_t i = 1; i < argc; i++){
        if (std::stoi(argv[i]) < 0){
            throw std::runtime_error("Negative input");
        }
        cnt[i - 1] = static_cast<unsigned int>(std::stoi(argv[i]));
    }
    for (unsigned int i = 0; i < argc - 1; i++){
        gen(cnt[i], "in.txt");
        compress("in.txt", "out.txt");
        decompress("out.txt", "finish.txt");
        check("in.txt", "finish.txt");
    }
}
