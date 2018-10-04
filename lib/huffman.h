#ifndef HUFFMAN2_HUFFMAN_H
#define HUFFMAN2_HUFFMAN_H

#include "huff_tree.h"
#include <map>

namespace huffman {
    static const int BLOCK_SIZE = 8192;

    void compress(std::istream &, std::ostream &, const bool);

    void decompress(std::istream &, std::ostream &);
 }

#endif //HUFFMAN2_HUFFMAN_H
