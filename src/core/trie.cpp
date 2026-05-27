#include "trie.h"

#include <iostream>

Trie::Trie()
    : trie_size_(0), first_(nullptr)
{
}

Trie::~Trie()
{
    destroy(first_);
}

void Trie::clear()
{
    destroy(first_);
    first_ = nullptr;
    trie_size_ = 0;
}

Trie::Node *Trie::create_node(char ch) const
{
    Node *node = new Node;
    node->ch = ch;
    node->right = nullptr;
    node->down = nullptr;
    node->posting_list = nullptr;
    return node;
}

void Trie::destroy(Node *node)
{
    if (node == nullptr)
    {
        return;
    }

    destroy(node->down);
    destroy(node->right);
    delete node->posting_list;
    delete node;
}

void Trie::insert_word(const std::string &word, std::size_t doc_id)
{
    if (word.empty())
    {
        return;
    }

    Node *parent_node = nullptr;
    Node *start_node = first_;

    for (std::size_t i = 0; i < word.size(); ++i)
    {
        Node *previous = nullptr;
        Node *current = start_node;
        Node *created = nullptr;
        bool reset_parent_child = false;

        if (current == nullptr)
        {
            created = create_node(word[i]);
            reset_parent_child = true;
        }
        else
        {
            while (true)
            {
                if (current == nullptr)
                {
                    created = create_node(word[i]);
                    previous->right = created;
                    break;
                }

                if (current->ch == word[i])
                {
                    parent_node = current;
                    start_node = current->down;
                    break;
                }

                if (current->ch < word[i])
                {
                    previous = current;
                    current = current->right;
                    continue;
                }

                created = create_node(word[i]);
                created->right = current;

                if (previous == nullptr)
                {
                    reset_parent_child = true;
                }
                else
                {
                    previous->right = created;
                }

                break;
            }
        }

        if (created != nullptr)
        {
            if (reset_parent_child)
            {
                if (parent_node != nullptr)
                {
                    parent_node->down = created;
                }
                else
                {
                    first_ = created;
                }
            }

            ++trie_size_;
            parent_node = created;
            start_node = created->down;
        }
    }

    if (parent_node->posting_list == nullptr)
    {
        parent_node->posting_list = new PostingList();
    }

    parent_node->posting_list->update(word, doc_id);
}

PostingList *Trie::search_word(const std::string &query) const
{
    Node *node = find_last_node(query);

    if (node == nullptr)
    {
        return nullptr;
    }

    return node->posting_list;
}

std::vector<std::string> Trie::autocomplete(const std::string &prefix, std::size_t limit) const
{
    std::vector<std::string> words;

    if (prefix.empty() || limit == 0)
    {
        return words;
    }

    Node *prefix_node = find_last_node(prefix);
    if (prefix_node == nullptr)
    {
        return words;
    }

    std::string current = prefix;
    if (prefix_node->posting_list != nullptr)
    {
        words.push_back(prefix);
    }

    collect_words(prefix_node->down, current, words, limit);
    return words;
}

void Trie::collect_words(Node *node, std::string &current, std::vector<std::string> &words,
                         std::size_t limit) const
{
    if (node == nullptr || words.size() >= limit)
    {
        return;
    }

    Node *cursor = node;
    while (cursor != nullptr && words.size() < limit)
    {
        current.push_back(cursor->ch);

        if (cursor->posting_list != nullptr)
        {
            words.push_back(current);
        }

        collect_words(cursor->down, current, words, limit);
        current.pop_back();
        cursor = cursor->right;
    }
}

Trie::Node *Trie::find_last_node(const std::string &query) const
{
    if (query.empty())
    {
        return nullptr;
    }

    Node *start_node = first_;
    Node *last_char_node = nullptr;

    for (std::size_t i = 0; i < query.size(); ++i)
    {
        Node *current = start_node;

        while (true)
        {
            if (current == nullptr)
            {
                return nullptr;
            }

            if (current->ch == query[i])
            {
                last_char_node = current;
                start_node = current->down;
                break;
            }

            if (current->ch > query[i])
            {
                return nullptr;
            }

            current = current->right;
        }
    }

    return last_char_node;
}

void Trie::print(bool full_print, std::ostream &out) const
{
    if (first_ == nullptr)
    {
        out << "Index is empty.\n";
        return;
    }

    print_subtrie(first_, full_print, out);
}

void Trie::print_subtrie(Node *node, bool full_print, std::ostream &out) const
{
    if (node == nullptr)
    {
        return;
    }

    if (node->posting_list != nullptr)
    {
        node->posting_list->print(full_print, out);
    }

    print_subtrie(node->down, full_print, out);
    print_subtrie(node->right, full_print, out);
}

std::size_t Trie::size() const
{
    return trie_size_;
}
