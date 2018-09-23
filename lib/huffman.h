#ifndef HUFFMAN2_HUFFMAN_H
#define HUFFMAN2_HUFFMAN_H

#include "huff_tree.h"
#include <map>

namespace huffman {
    static const int BLOCK_SIZE = 8192;

    void compress(std::istream &, std::ostream &);

    void decompress(std::istream &, std::ostream &);
/*
    void error();

    void writeCompressedText(std::istream &in, std::ostream &out, std::pair<uint64_t, int> *);

    template<typename T>
    void readOneNumber(std::istream &, T &);

    void writeSymbFreq(std::ostream &, uint64_t *);

    void pushCodeInUINT64(uint64_t &, uint32_t &, int, uint64_t);

    bool trySymb(std::ostream &out, std::map<std::pair<uint64_t, int>, unsigned char> &map_table,
                 std::pair<uint64_t, int> bits);

    const uint32_t CNT_ALPH_SYMB = 256;

    uint64_t fillFreq(std::istream &in, uint64_t *const freq);
*/
 }

#endif //HUFFMAN2_HUFFMAN_H
