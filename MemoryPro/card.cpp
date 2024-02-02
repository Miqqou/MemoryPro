#include "card.hh"

Card::Card():
    id_(EMPTY_STRING)
{
}

Card::Card(const std::string s):
    id_(s)
{
}

void Card::set_id(const std::string s)
{
    id_ = s;
}

std::string Card::get_id() const
{
    return id_;
}
