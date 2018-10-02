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

        void error(std::string const &errorText) {
            throw std::runtime_error(std::string(errorText));
        }

        template<typename T>
        void writeOneNumber(std::ostream &out, T const &v) {
            for (size_t i = sizeof(v) - 1; i + 1 >= 1; i--) {
                out << static_cast<uint8_t >((v >> (i * 8)) & 255);
            }
        }

        void writeSymbFreq(std::ostream &out, uint64_t *freq) {
            for (size_t i = 0; i < CNT_ALPH_SYMB; i++) {
                if (freq[i] > 0) {
                    writeOneNumber(out, static_cast<unsigned char>(i));
                    writeOneNumber(out, freq[i]);
                }
            }
        }

        template<typename T>
        void readOneNumber(std::istream &in, T &v) {
            v = 0;
            for (size_t i = sizeof(T) - 1; i + 1 >= 1; i--) {
                char tmp1;
                in.get(tmp1);
                auto tmp = static_cast<unsigned char>((tmp1 < 0) ? 256 + tmp1 : tmp1);
                v <<= 8;
                v += tmp;
            }
        }

        template<typename T>
        uint64_t readBlock(std::istream &in, T *buf, size_t const count = BLOCK_SIZE) {
            int64_t len = 0;
            size_t const size = count * sizeof(T);
            try {
                in.read(reinterpret_cast<char *>(buf), size);
                len = static_cast<int64_t >(in.gcount());
            }
            catch (const std::exception &e) {
                error("Can't read block");
            }
            if (len < 0) {
                error("Can't read block");
            }
            return static_cast<uint64_t>(len);
        }

        void pushCodeInUINT64(uint64_t &tmp, uint32_t &size, const int codeSize, const uint64_t code) {
            tmp <<= codeSize;
            tmp += code;
            size += codeSize;
        }

        void writeCompressedText(std::istream &in, std::ostream &out, std::pair<uint64_t, int> *table) {
            uint64_t tmp = 0;
            std::ios::fixed;
            out.precision(sizeof(tmp) * 8);
            uint32_t edge = 0;
            uint32_t size = 0;
            uint64_t len;
            unsigned char buf[BLOCK_SIZE];

            do {
                len = readBlock(in, buf);

                for (size_t i = 0; i < len; ++i) {
                    if (size + table[buf[i]].second <= sizeof(tmp) * 8) {
                        pushCodeInUINT64(tmp, size, table[buf[i]].second, table[buf[i]].first);
                    } else {
                        uint32_t freeCap = sizeof(tmp) * 8 - size;
                        edge = table[buf[i]].second - freeCap;
                        pushCodeInUINT64(tmp, size, freeCap, table[buf[i]].first >> (table[buf[i]].second - freeCap));
                        writeOneNumber(out, tmp);
                        tmp = table[buf[i]].first & ((static_cast<uint64_t >(1) << (edge + 1)) - 1);
                        size = edge;
                    }
                }
            } while (len > 0);

            if (size != 0) {
                tmp <<= (sizeof(tmp) * 8 - size);
                writeOneNumber(out, tmp);
            }
        }

        uint64_t fillFreq(std::istream &in, uint64_t *const freq) {
            unsigned char buf[BLOCK_SIZE];
            uint64_t full_len = 0, len;

            do {
                len = readBlock(in, buf);
                full_len += len;
                for (size_t i = 0; i < len; i++) {
                    freq[buf[i]]++;
                }
            } while (len != 0);

            return full_len;
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

        writeOneNumber(out, cnt_symb);

        writeOneNumber(out, full_len);

        writeSymbFreq(out, freq);


        in.clear();
        in.seekg(0, std::ios::beg);

        writeCompressedText(in, out, table);
    }

    void decompress(std::istream &in, std::ostream &out) {
        uint32_t cnt_syb;
        readOneNumber(in, cnt_syb);
        uint64_t siz;

        readOneNumber(in, siz);
        uint64_t freq[CNT_ALPH_SYMB];
        std::fill(freq, freq + CNT_ALPH_SYMB, 0);

        for (uint32_t i = 0; i < cnt_syb; i++) {
            unsigned char symb = 0;
            uint64_t cnt = 0;
            readOneNumber(in, symb);
            readOneNumber(in, cnt);

            freq[symb] = cnt;
        }

        huff_tree tree;
        huff_tree::node *root = tree.makeTree(freq);
        huff_tree::node *now = root;

        uint64_t check_sum[CNT_ALPH_SYMB];
        std::fill(check_sum, check_sum + CNT_ALPH_SYMB, 0);


        while (siz > 0) {
            unsigned char buf[BLOCK_SIZE];
            uint64_t len;
            len = readBlock(in, buf);

            if (len == 0) {
                error();
            }

            for (size_t i = 0; i < len && siz > 0; i++) {
                for (size_t cnt = sizeof(buf[i]) * 8 - 1; cnt + 1 > 0 && siz > 0; cnt--) {
                    if (now->right && (buf[i] >> cnt) & 1) {
                        now = now->right;
                    } else if (now->left) {
                        now = now->left;
                    }
                    if (now->num != huff_tree::FALSE_CHAR) {
                        out << static_cast<unsigned char>(now->num);
                        check_sum[now->num]++;
                        now = root;
                        siz--;
                    }
                }
            }
        }
        for (size_t i = 0; i < CNT_ALPH_SYMB; i++) {
            if (check_sum[i] != freq[i]) {
                throw std::runtime_error(std::string("Error in check decoded file. Checksum freqs are not equals"));
            }
        }
    }
}
