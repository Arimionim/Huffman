//
// Created by Arslan on 12.09.2018.
//

#include <queue>
#include <string>

#ifndef HUFFMAN2_HUFF_TREE_H
#define HUFFMAN2_HUFF_TREE_H

struct huff_tree {
    void makeTable(uint64_t a[], std::pair<uint64_t, int> *);

private:
    struct node {
        node *left;
        node *right;
        unsigned int num;
    };

    unsigned int FALSE_CHAR = 300;

    void dfs(node *, std::pair<uint64_t, int> *, std::pair<uint64_t, int> const &);

    std::priority_queue<std::pair<std::pair<uint64_t, uint32_t>, node *>> queue;
};

#endif //HUFFMAN2_HUFF_TREE_H
