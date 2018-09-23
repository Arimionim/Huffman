#include <iostream>
#include "huffman.h"
#include <fstream>
#include <cstring>

namespace huffman {

    namespace {
        const uint32_t CNT_ALPH_SYMB = 256;

        void error() {
            throw std::runtime_error(std::string("Input is invalid"));
        }

        void writeSymbFreq(std::ostream &out, uint64_t *freq) {
            for (size_t i = 0; i < CNT_ALPH_SYMB; i++) {
                if (freq[i] > 0) {
                    out.write(reinterpret_cast<const char *>(&i), sizeof(char));
                    out.write(reinterpret_cast<const char *>(&freq[i]), sizeof(freq[i]));
                }
            }
        }

        void pushCodeInUINT64(uint64_t &tmp, uint32_t &size, const int codeSize, const uint64_t code) {
            tmp <<= codeSize;
            tmp += code;
            size += codeSize;
        }

        void writeCompressedText(std::istream &in, std::ostream &out, std::pair<uint64_t, int> *table) {
            uint64_t tmp = 0;
            uint32_t edge = 0;
            uint32_t size = 0;
            uint32_t len;
            unsigned char buf[BLOCK_SIZE];

            do {
                (in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE));
                len = static_cast<size_t>(in.gcount());

                for (size_t i = 0; i < len; ++i) {
                    if (size + table[buf[i]].second <= sizeof(tmp) * 8) {
                        pushCodeInUINT64(tmp, size, table[buf[i]].second, table[buf[i]].first);
                    } else {
                        tmp <<= (sizeof(tmp) * 8 - size);
                        if (size != sizeof(tmp) * 8)
                            tmp += (table[buf[i]].first >> (table[buf[i]].second - (sizeof(tmp) * 8 - size)));
                        edge = table[buf[i]].second - (sizeof(tmp) * 8 - size);
                        out.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
                        tmp = 0;
                        for (size_t j = 0; j < edge; j++) {
                            tmp += table[buf[i]].first & (1 << j);
                        }
                        size = edge;
                    }
                }
            } while (len > 0);
            if (size != 0) {
                tmp <<= (sizeof(tmp) * 8 - size);
                out.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
            }
        }

        uint64_t fillFreq(std::istream &in, uint64_t *const freq) {
            unsigned char buf[BLOCK_SIZE];
            uint64_t len, full_len = 0;

            do {
                (in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE));
                len = static_cast<size_t>(in.gcount());
                full_len += len;

                for (size_t i = 0; i < len; i++) {
                    freq[buf[i]]++;
                }
            } while (len != 0);
            uint64_t check = 0;

            for (uint32_t i = 0; i < CNT_ALPH_SYMB; i++) {
                check += freq[i];
            }
            if (check != full_len) {
                throw std::runtime_error(std::string("Error in fillFreq. Checksum not equal to full length"));
            }
            return full_len;
        }

        template<typename T>
        void readOneNumber(std::istream &in, T &v) {
            in.read(reinterpret_cast<char *>(&v), sizeof(v));
            if (static_cast<size_t >(in.gcount()) < sizeof(v)) {
                error();
            }
        }

        bool trySymb(std::ostream &out, std::map<std::pair<uint64_t, int>, unsigned char> &map_table,
                     std::pair<uint64_t, int> bits) {
            if (map_table.find(bits) != map_table.end()) {
                out.write(reinterpret_cast<const char *>(&map_table[bits]), sizeof(map_table[bits]));
                return true;
            }
            return false;
        }
    }

    void compress(std::istream &in, std::ostream &out) {
        uint64_t freq[CNT_ALPH_SYMB];
        std::fill(freq, freq + CNT_ALPH_SYMB, 0);

        uint64_t full_len = fillFreq(in, freq);

        std::pair<uint64_t, int> table[CNT_ALPH_SYMB];
        huff_tree tree;
        tree.makeTable(freq, table);

        uint32_t cnt_symb = 0;

        for (uint64_t i : freq) {
            cnt_symb += (i != 0);
        }

        out.write(reinterpret_cast<const char *>(&cnt_symb), sizeof(cnt_symb));

        writeSymbFreq(out, freq);

        out.write(reinterpret_cast<const char *>(&full_len), sizeof(full_len));

        in.clear();
        in.seekg(0, std::ios::beg);

        writeCompressedText(in, out, table);
    }

    void decompress(std::istream &in, std::ostream &out) {
        uint32_t cnt_syb;
        readOneNumber(in, cnt_syb);
        uint64_t freq[CNT_ALPH_SYMB];
        std::fill(freq, freq + CNT_ALPH_SYMB, 0);

        for (uint32_t i = 0; i < cnt_syb; i++) {
            unsigned char symb;
            uint64_t cnt;
            readOneNumber(in, symb);
            readOneNumber(in, cnt);

            freq[symb] = cnt;
        }

        std::pair<uint64_t, int> table[CNT_ALPH_SYMB];
        huff_tree tree;
        tree.makeTable(freq, table);
        std::map<std::pair<uint64_t, int>, unsigned char> map_table;

        for (uint32_t i = 0; i < CNT_ALPH_SYMB; i++) {
            if (freq[i] != 0) {
                map_table[table[i]] = static_cast<unsigned char>(i);
            }
        }
        uint64_t siz;

        readOneNumber(in, siz);

        std::pair<uint64_t, int> bits = {0, 0};


        while (siz > 0) {
            uint64_t buf[BLOCK_SIZE / sizeof(uint64_t)];
            in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE);
            uint64_t len;
            len = static_cast<uint32_t>(in.gcount());

            if (len == 0) {
                error();
            }
            len /= sizeof(uint64_t);

            for (size_t i = 0; i < len && siz > 0; i++) {
                for (size_t cnt = sizeof(buf[i]) * 8 - 1; cnt + 1 > 0 && siz > 0; cnt--) {
                    bits.first <<= 1;
                    bits.first += (buf[i] >> cnt) & 1;
                    bits.second++;
                    if (trySymb(out, map_table, bits)) {
                        bits = {0, 0};
                        siz--;
                    }
                }
            }
        }
    }
}
