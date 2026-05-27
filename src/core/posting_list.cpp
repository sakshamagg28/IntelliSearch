#include "posting_list.h"

#include <iostream>

PostingList::Node::Node(std::size_t id)
    : doc_id(id), frequency(1), next(nullptr)
{
}

PostingList::PostingList()
    : first_(nullptr), document_frequency_(0), term_()
{
}

void PostingList::update(const std::string &term, std::size_t doc_id)
{
    if (term_.empty())
    {
        term_ = term;
    }

    if (!first_)
    {
        first_.reset(new Node(doc_id));
        ++document_frequency_;
        return;
    }

    Node *previous = nullptr;
    Node *current = first_.get();

    while (current != nullptr && current->doc_id < doc_id)
    {
        previous = current;
        current = current->next.get();
    }

    if (current != nullptr && current->doc_id == doc_id)
    {
        ++current->frequency;
        return;
    }

    std::unique_ptr<Node> new_node(new Node(doc_id));
    if (previous == nullptr)
    {
        new_node->next = std::move(first_);
        first_ = std::move(new_node);
    }
    else
    {
        new_node->next = std::move(previous->next);
        previous->next = std::move(new_node);
    }

    ++document_frequency_;
}

void PostingList::print(bool full_print, std::ostream &out) const
{
    if (!full_print)
    {
        out << term_ << "  " << document_frequency_ << '\n';
        return;
    }

    out << "term: '" << term_ << "'  df=" << document_frequency_ << "  ";

    const Node *current = first_.get();
    while (current != nullptr)
    {
        out << "[" << current->doc_id << ", " << current->frequency << "] ";
        current = current->next.get();
    }

    out << '\n';
}

std::size_t PostingList::document_frequency() const
{
    return document_frequency_;
}

std::size_t PostingList::term_frequency(std::size_t doc_id) const
{
    const Node *current = first_.get();

    while (current != nullptr)
    {
        if (current->doc_id == doc_id)
        {
            return current->frequency;
        }

        if (current->doc_id > doc_id)
        {
            return 0;
        }

        current = current->next.get();
    }

    return 0;
}

std::vector<std::size_t> PostingList::document_ids() const
{
    std::vector<std::size_t> ids;
    const Node *current = first_.get();

    while (current != nullptr)
    {
        ids.push_back(current->doc_id);
        current = current->next.get();
    }

    return ids;
}

std::vector<PostingList::Entry> PostingList::entries() const
{
    std::vector<Entry> values;
    const Node *current = first_.get();

    while (current != nullptr)
    {
        values.push_back({current->doc_id, current->frequency});
        current = current->next.get();
    }

    return values;
}

const std::string &PostingList::term() const
{
    return term_;
}
