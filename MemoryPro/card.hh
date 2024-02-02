#ifndef CARD_HH
#define CARD_HH

#include <string>

const std::string EMPTY_STRING = ".";

/*
 * Represents a single card in the game.
 */
class Card
{
public:
    Card();

    // Creates a hidden card with the given string.
    Card(const std::string s);

    void set_id(const std::string s);
    std::string get_id() const;


    // TODO: picture data here

private:
    std::string id_;
};

#endif // CARD_HH
