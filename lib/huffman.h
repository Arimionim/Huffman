#ifndef HUFFMAN2_HUFFMAN_H
#define HUFFMAN2_HUFFMAN_H

#include "huff_tree.h"

namespace huffman {
    static const int BLOCK_SIZE = 8192;

    void compress(std::istream&, std::ostream&);
    void decompress(std::istream&, std::ostream&);

    void error();

    template<typename T>
    void read(std::istream&, T &);
}

#endif //HUFFMAN2_HUFFMAN_H
