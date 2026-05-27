#ifndef INTELLISEARCH_POSTING_LIST_H
#define INTELLISEARCH_POSTING_LIST_H

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class PostingList
{
public:
    struct Entry
    {
        std::size_t doc_id;
        std::size_t frequency;
    };

    PostingList();

    void update(const std::string &term, std::size_t doc_id);
    void print(bool full_print, std::ostream &out) const;

    std::size_t document_frequency() const;
    std::size_t term_frequency(std::size_t doc_id) const;
    std::vector<std::size_t> document_ids() const;
    std::vector<Entry> entries() const;
    const std::string &term() const;

private:
    struct Node
    {
        explicit Node(std::size_t id);

        std::size_t doc_id;
        std::size_t frequency;
        std::unique_ptr<Node> next;
    };

    std::unique_ptr<Node> first_;
    std::size_t document_frequency_;
    std::string term_;
};

#endif
