#include "huff_tree.h"
#include <iostream>

void huff_tree::makeTable(unsigned int freq[], std::string * const m){

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
        dfs(queue.top().second, m, "");
    }

}

void huff_tree::dfs(node *v, std::string *m, std::string const &key){
    if (!v){
        return;
    }
    if (v->num != 300){
        m[v->num] = key;
        if (key == ""){
            m[v->num] = "1";
        }
    }
    dfs(v->left, m, key + '0');
    dfs(v->right, m, key + '1');
    delete(v);
}