#include "huff_tree.h"
#include <iostream>

void huff_tree::makeTable(uint64_t freq[], std::pair<uint64_t, int> *const m) {

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
    if (!queue.empty()) {
        dfs(queue.top().second, m, {0, 0});
    }

}

void huff_tree::dfs(node *v, std::pair<uint64_t, int> *m, std::pair<uint64_t, int> const &key) {
    if (!v) {
        return;
    }
    if (v->num != FALSE_CHAR) {
        m[v->num] = key;
        if (key.second == 0) {
            m[v->num] = {0, 1};
        }
    }
    dfs(v->left, m, {key.first * 2, key.second + 1});
    dfs(v->right, m, {key.first * 2 + 1, key.second + 1});
    delete (v);
}