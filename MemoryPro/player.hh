
#ifndef PLAYER_HH
#define PLAYER_HH

#include <string>

/*
 * Represents a single player in the game.
*/
class Player
{
public:
    // Creates a player with the given name.
    Player(const std::string& name);

    // Returns the name of the player.
    std::string get_name() const;

    // Returns the number of pairs collected by the player so far.
    int number_of_pairs() const;

    // Adds 1 pair for the player.
    void add_pair();

private:
    std::string name_;
    int pairs_ = 0;    // guessed pairs by the player
};

#endif // PLAYER_HH
