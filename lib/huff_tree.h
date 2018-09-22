//
// Created by Arslan on 12.09.2018.
//

#include <queue>
#include <string>

#ifndef HUFFMAN2_HUFF_TREE_H
#define HUFFMAN2_HUFF_TREE_H

struct huff_tree{
    void makeTable(unsigned int a[], std::pair<int, int>*);

private:
    struct node{
        node *left;
        node *right;
        unsigned int num;
    };

    void dfs(node *, std::pair<int, int> *, std::pair<int, int> const &);

    std::priority_queue<std::pair<std::pair<unsigned int, unsigned int>, node*>> queue;
};

#endif //HUFFMAN2_HUFF_TREE_H
