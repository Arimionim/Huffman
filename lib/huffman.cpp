#include <iostream>
#include "huffman.h"
#include <unordered_map>
#include <fstream>


namespace huffman {
    void compress(std::istream &in, std::ostream &out) {
        unsigned int freq[256];
        unsigned char buf[BLOCK_SIZE];

        for (int i = 0; i < 256; i++) {
            freq[i] = 0;
        }

        unsigned int len, full_len = 0;

        do {
            (in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE));
            len = static_cast<size_t>(in.gcount());
            full_len += len;

            for (size_t i = 0; i < len; i++) {
                freq[buf[i]]++;
            }
        } while (len != 0);

        std::string table[256];
        huff_tree tree;
        tree.makeTable(freq, table);

        unsigned int cnt_symb = 0;
        for (size_t i = 0; i < 256; i++) {
            cnt_symb += (freq[i] != 0);
        }

        out.write(reinterpret_cast<const char *>(&cnt_symb), sizeof(cnt_symb));

        for (size_t i = 0; i < 256; i++) {
            if (freq[i] > 0) {
                out.put(static_cast<char>(i));
                out.write(reinterpret_cast<const char *>(&freq[i]), sizeof(freq[i]));
            }
        }

        out.write(reinterpret_cast<const char *>(&full_len), sizeof(full_len));

        std::string bits;
        in.clear();
        in.seekg(0, std::ios::beg);

        do {
            (in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE));
            len = static_cast<size_t>(in.gcount());
            unsigned int edge = 0;
            for (size_t i = 0; i < len; i++) {
                bits += table[buf[i]];
            }
            unsigned int tmp = 0;

            for (size_t i = 0; i < bits.size(); ++i) {
                tmp *= 2;
                tmp += bits[i] == '1';
                if ((i + 1) % 32 == 0) {
                    out.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
                    edge = i + 1;
                }
            }
            bits = bits.substr(edge, bits.size() - edge);
        } while (len > 0);

        if (!bits.empty()) {
            unsigned int tmp = 0;
            for (size_t i = 0; i < 32; i++) {
                tmp *= 2;
                tmp += i < bits.size() ? (bits[i] == '1') : 0;
            }
            out.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
        }
    }

    template<typename T>
    void read(std::istream &in, T &v) {
        in.read(reinterpret_cast<char *>(&v), sizeof(v));
        if (static_cast<size_t >(in.gcount()) < sizeof(v)) {
            error();
        }
    }

    void decompress(std::istream &in, std::ostream &out) {
        unsigned int cnt_syb;
        read(in, cnt_syb);
        unsigned int freq[256];
        for (int i = 0; i < 256; i++) {
            freq[i] = 0;
        }

        for (unsigned int i = 0; i < cnt_syb; i++) {
            unsigned char symb;
            unsigned int cnt;
            read(in, symb);
            read(in, cnt);

            freq[symb] = cnt;
        }


        std::string table[256];
        huff_tree tree;
        tree.makeTable(freq, table);

        std::unordered_map<std::string, unsigned char > map_table;
        for (size_t i = 0; i < 256; i++) {
            if (freq[i] != 0) {
                map_table[table[i]] = i;
            }
        }
        unsigned int siz;

        read(in, siz);

        std::string bits;
        while (siz > 0) {
            unsigned int buf[BLOCK_SIZE / 4];
            in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE);
            unsigned int len;
            len = static_cast<unsigned int>(in.gcount());
            
            if (len == 0) {
                error();
            }
            len /= 4;
            std::string ans;
            for (size_t i = 0; i < len && siz > 0; i++) {
                for (size_t cnt = 31; cnt + 1 > 0 && siz > 0; cnt--) {
                    bits += ((buf[i] >> cnt) & 1) ? "1" : "0";
                    if (map_table.find(bits) != map_table.end()) {
                        out.write(reinterpret_cast<const char *>(&map_table[bits]), sizeof(map_table[bits]));
                        bits = "";
                        siz--;
                    }
                }
            }
        }
    }


    void error() {
        throw std::runtime_error(std::string("Input is invalid"));
    }

}