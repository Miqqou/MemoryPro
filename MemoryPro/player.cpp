#include "player.hh"

Player::Player(const std::string &name):
    name_(name)
{

}

std::string Player::get_name() const
{
    return name_;
}

int Player::number_of_pairs() const
{
    return pairs_;
}

void Player::add_pair()
{
    pairs_ += 1;
}
