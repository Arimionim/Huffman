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
        void writeOneNumber(std::ostream &out, T const &v, bool reversed = false) {
            T mask = 255;
            if (!reversed) {
                for (size_t i = sizeof(v) - 1; i + 1 >= 1; i--) {
                    out << static_cast<uint8_t >((v >> (i * 8)) & mask);
                }
            } else {
                for (size_t i = 0; i < sizeof(v); i++) {
                    out << static_cast<uint8_t >((v >> (i * 8)) & mask);
                }
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
        uint64_t readBlock(std::istream &in, T *buf) {
            (in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE / sizeof(T)));
            auto len = static_cast<uint64_t >(in.gcount());
            if (len < 0) {
                error("Can't read block");
            }
            return len;
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
            uint64_t len;
            unsigned char buf[BLOCK_SIZE];

            do {
                len = readBlock(in, buf);

                for (size_t i = 0; i < len; ++i) {
                    if (size + table[buf[i]].second <= sizeof(tmp) * 8) {
                        pushCodeInUINT64(tmp, size, table[buf[i]].second, table[buf[i]].first);
                    } else {
                        tmp <<= (sizeof(tmp) * 8 - size);
                        if (size != sizeof(tmp) * 8) {
                            tmp += (table[buf[i]].first >> (table[buf[i]].second - (sizeof(tmp) * 8 - size)));
                        }
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
            uint64_t full_len = 0, len;

            do {
                len = readBlock(in, buf);
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

        std::pair<uint64_t, int> table[CNT_ALPH_SYMB];
        huff_tree tree;
        tree.makeTable(freq, table);
        std::map<std::pair<uint64_t, int>, unsigned char> map_table;

        for (uint32_t i = 0; i < CNT_ALPH_SYMB; i++) {
            if (freq[i] != 0) {
                map_table[table[i]] = static_cast<unsigned char>(i);
            }
        }


        std::pair<uint64_t, int> bits = {0, 0};

        uint64_t check_sum[CNT_ALPH_SYMB];
        std::fill(check_sum, check_sum + CNT_ALPH_SYMB, 0);


        while (siz > 0) {
            uint64_t buf[BLOCK_SIZE / sizeof(uint64_t)];
            uint64_t len;
            len = readBlock(in, buf);

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
                        check_sum[map_table[bits]]++;
                        bits = {0, 0};
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
