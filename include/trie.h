#ifndef INTELLISEARCH_TRIE_H
#define INTELLISEARCH_TRIE_H

#include "posting_list.h"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

class Trie
{
public:
    Trie();
    ~Trie();

    Trie(const Trie &) = delete;
    Trie &operator=(const Trie &) = delete;

    void clear();
    void insert_word(const std::string &word, std::size_t doc_id);
    PostingList *search_word(const std::string &query) const;
    std::vector<std::string> autocomplete(const std::string &prefix, std::size_t limit) const;
    void print(bool full_print, std::ostream &out) const;
    std::size_t size() const;

private:
    struct Node
    {
        char ch;
        Node *right;
        Node *down;
        PostingList *posting_list;
    };

    Node *create_node(char ch) const;
    void destroy(Node *node);
    void print_subtrie(Node *node, bool full_print, std::ostream &out) const;
    Node *find_last_node(const std::string &query) const;
    void collect_words(Node *node, std::string &current, std::vector<std::string> &words,
                       std::size_t limit) const;

    std::size_t trie_size_;
    Node *first_;
};

#endif
