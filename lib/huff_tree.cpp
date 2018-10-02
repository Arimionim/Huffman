#include "huff_tree.h"
#include <iostream>

void huff_tree::makeTable(uint64_t freq[], std::pair<uint64_t, int> *const m) {
    buildQueue(freq);
    if (!queue.empty()) {
        fillTable(queue.top().second, m, {0, 0});
        deleteTree(queue.top().second);
    }
}

huff_tree::node *huff_tree::makeTree(uint64_t freq[]) {
    buildQueue(freq);
    return queue.top().second;
}

void huff_tree::deleteTree(huff_tree::node *node) {
    if (!node){
        return;
    }
    deleteTree(node->left);
    deleteTree(node->right);
    delete(node);
}

void huff_tree::buildQueue(uint64_t freq[]) {
    uint32_t cnt = 0;
    for (uint32_t i = 0; i < 256; i++) {
        if (freq[i] != 0) {
            ++cnt;
            node *tmp = new node({nullptr, nullptr, i});
            huff_tree::queue.push({{UINT64_MAX - freq[i], UINT32_MAX - i}, tmp});
        }
    }
    while (queue.size() > 1) {
        auto a = queue.top();
        queue.pop();
        auto b = queue.top();
        queue.pop();
        node *tmp = new node({a.second, b.second, FALSE_CHAR});
        queue.push({{a.first.first + b.first.first, a.first.second * 256 + b.first.second}, tmp});
    }
}


void huff_tree::fillTable(node *v, std::pair<uint64_t, int> *m, std::pair<uint64_t, int> const &key) {
    if (!v) {
        return;
    }
    if (v->num != FALSE_CHAR) {
        m[v->num] = key;
    }
    fillTable(v->left, m, {key.first * 2, key.second + 1});
    fillTable(v->right, m, {key.first * 2 + 1, key.second + 1});
}

