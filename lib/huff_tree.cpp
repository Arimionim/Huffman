#include "huff_tree.h"
#include <iostream>

void huff_tree::makeTable(unsigned int freq[], std::pair<int, int> * const m){

    unsigned int cnt = 0;
    for (unsigned int i = 0; i < 256; i++){
        if (freq[i] != 0) {
            ++cnt;
            node *tmp = new node({nullptr, nullptr, i});
            huff_tree::queue.push({{-freq[i], -i}, tmp});
        }
    }
    while (queue.size() > 1) {
        auto a = queue.top();
        queue.pop();
        auto b = queue.top();
        queue.pop();
        node *tmp = new node({a.second, b.second, 300});
        queue.push({{a.first.first + b.first.first, a.first.second * 256 + b.first.second}, tmp});
    }
    if (queue.size() > 0) {
        dfs(queue.top().second, m, {0, 0});
    }

}

void huff_tree::dfs(node *v, std::pair<int, int> *m, std::pair<int, int> const &key){
    if (!v){
        return;
    }
    if (v->num != 300){
        m[v->num] = key;
        if (key.second == 0){
            m[v->num] = {0, 1};
        }
    }
    std::pair<int, int> tmp = {64, 7};
    if (key == tmp){
        m[v->num].second--;
        m[v->num].second++;
    }
    dfs(v->left, m, {key.first * 2, key.second + 1});
    dfs(v->right, m, {key.first * 2 + 1, key.second + 1});
    delete(v);
}