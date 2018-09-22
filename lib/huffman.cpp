#include <iostream>
#include "huffman.h"
#include <map>
#include <fstream>
#include <cstring>

namespace huffman {
    void compress(std::istream &in, std::ostream &out) {
        uint32_t freq[256];
        unsigned char buf[BLOCK_SIZE];

        std::memset(freq, 0, 256 * sizeof(uint32_t));

        uint32_t len, full_len = 0;

        do {
            (in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE));
            len = static_cast<size_t>(in.gcount());
            full_len += len;

            for (size_t i = 0; i < len; i++) {
                freq[buf[i]]++;
            }
        } while (len != 0);

        std::pair<int, int> table[256];
        huff_tree tree;
        tree.makeTable(freq, table);

        uint32_t cnt_symb = 0;
        for (size_t i = 0; i < 256; i++) {
            cnt_symb += (freq[i] != 0);
        }

        out.write(reinterpret_cast<const char *>(&cnt_symb), sizeof(cnt_symb));

        for (size_t i = 0; i < 256; i++) {
            if (freq[i] > 0) {
                out.write(reinterpret_cast<const char *>(&i), sizeof(char));
                out.write(reinterpret_cast<const char *>(&freq[i]), sizeof(freq[i]));
            }
        }

        out.write(reinterpret_cast<const char *>(&full_len), sizeof(full_len));

        in.clear();
        in.seekg(0, std::ios::beg);


        uint32_t tmp = 0;
        uint32_t edge = 0;
        uint32_t size = 0;

        do {
            (in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE));
            len = static_cast<size_t>(in.gcount());

            for (size_t i = 0; i < len; ++i) {
                if (i == 69){
                    i--;
                    i++;
                }
                if (size + table[buf[i]].second <= 32){
                    tmp <<= table[buf[i]].second;
                    tmp += table[buf[i]].first;
                    size += table[buf[i]].second;
                }
                else {
                    tmp <<= (32 - size);
                    if (size != 32)
                        tmp += (table[buf[i]].first >> (table[buf[i]].second - (32 - size)));
                    edge = table[buf[i]].second - (32 - size);
                    out.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
                    tmp = 0;
                    for (size_t j = 0; j < edge; j++){
                        tmp += table[buf[i]].first & (1 << j);
                    }
                    size = edge;
                }
            }
        } while (len > 0);
        if (size != 0){
            tmp <<= (32 - size);
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
        uint32_t cnt_syb;
        read(in, cnt_syb);
        uint32_t freq[256];
        std::memset(freq, 0, 256 * sizeof(uint32_t));


        for (uint32_t i = 0; i < cnt_syb; i++) {
            unsigned char symb;
            uint32_t cnt;
            read(in, symb);
            read(in, cnt);

            freq[symb] = cnt;
        }


        std::pair<int, int> table[256];
        huff_tree tree;
        tree.makeTable(freq, table);
        std::map<std::pair<int, int>, unsigned char> map_table;
        for (uint32_t i = 0; i < 256; i++) {
            if (freq[i] != 0) {
                map_table[table[i]] = static_cast<unsigned char>(i);
            }
        }
        uint32_t siz;

        read(in, siz);

        std::pair<int, int> bits = {0, 0};

        while (siz > 0) {
            uint32_t buf[BLOCK_SIZE / 4];
            in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE);
            uint32_t len;
            len = static_cast<uint32_t>(in.gcount());
            
            if (len == 0) {
                error();
            }
            len /= 4;
            std::string ans;
            for (size_t i = 0; i < len && siz > 0; i++) {
                for (size_t cnt = 31; cnt + 1 > 0 && siz > 0; cnt--) {
                    bits.first <<= 1;
                    bits.first += (buf[i] >> cnt) & 1;
                    bits.second++;
                    if (map_table.find(bits) != map_table.end()) {
                        ans += map_table[bits];
                        out.write(reinterpret_cast<const char *>(&map_table[bits]), sizeof(map_table[bits]));
                        bits = {0, 0};
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
