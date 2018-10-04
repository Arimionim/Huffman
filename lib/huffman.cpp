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
            size_t size = sizeof(T);
            for (size_t i = size - 1; i + 1 >= 1; --i) {
                char tmp1;
                in.get(tmp1);
                auto tmp = static_cast<unsigned char>((tmp1 < 0) ? 256 + tmp1 : tmp1);
                v <<= 8;
                v += tmp;
            }
        }

        template<typename T>
        uint64_t readBlock(std::istream &in, T *buf, uint64_t const count = BLOCK_SIZE) {
            int64_t len = 0;
            auto const size = static_cast<const size_t>(count * sizeof(T));
            try {
                int64_t last;
                do {
                    in.read(reinterpret_cast<char *>(buf + len), static_cast<std::streamsize>(size - len));
                    last = static_cast<int64_t >(in.gcount());
                    if (last < 0){
                        error();
                    }
                    len += last;
                } while (last != 0 && len < size);
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
                        out.write(reinterpret_cast<char * >(&tmp), sizeof(tmp));
                        //writeOneNumber(out, tmp); //it writes in big endian, use for test
                        tmp = table[buf[i]].first & ((static_cast<uint64_t >(1) << (edge + 1)) - 1);
                        size = edge;
                    }
                }
            } while (len > 0);

            if (size != 0) {
                tmp <<= (sizeof(tmp) * 8 - size);
                //writeOneNumber(out, tmp);
                out.write(reinterpret_cast<char* >(&tmp), sizeof(tmp));
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

        uint64_t countBytes(const uint64_t freq[256], std::pair<uint64_t, int> table[256]) {
            uint64_t cntBits = 0;
            for (int i = 0; i < 256; i++){
                cntBits += freq[i] * table[i].second;
            }
            return (cntBits % 8) ? (cntBits / 8 + 1) : (cntBits / 8);
        }

        void reverseUint64_t(uint64_t &num){
            unsigned char bytes[sizeof(uint64_t)];
            for (int j = 0; j < sizeof(uint64_t); j++){
                bytes[j] = static_cast<unsigned char>((num >> (j * 8)) & (255));
            }
            num = 0;
            for (unsigned char byte : bytes) {
                num <<= 8;
                num += byte;
            }
        }
    }

    // false = little endian, true = big endian
    void compress(std::istream &in, std::ostream &out, const bool endian = false) {
        writeOneNumber(out, endian);
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

        uint64_t totalBytes = countBytes(freq, table);

        writeOneNumber(out, totalBytes);

        in.clear();
        in.seekg(0, std::ios::beg);

        writeCompressedText(in, out, table);
    }

    void decompress(std::istream &in, std::ostream &out) {
        bool endian = false;
        readOneNumber(in, endian);
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

        uint64_t totalBytes = 0;
        readOneNumber(in, totalBytes);

        while (siz > 0) {
            uint64_t const NEW_BLOCK_SIZE = BLOCK_SIZE < totalBytes ? BLOCK_SIZE : totalBytes;
            totalBytes -= NEW_BLOCK_SIZE;

            uint64_t buf[NEW_BLOCK_SIZE / sizeof(uint64_t)];
            uint64_t len = readBlock(in, buf, NEW_BLOCK_SIZE / sizeof(uint64_t));

            if (len < NEW_BLOCK_SIZE){
                error();
            }

            len /= sizeof(uint64_t);

            if (endian) {
                for (int i = 0; i < len; i++) {
                   reverseUint64_t(buf[i]);
                }
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
