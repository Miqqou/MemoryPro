#ifndef CONFIG_HH
#define CONFIG_HH
#include <string>

/*
 * This header includes all configurable setings for the game.
*/
namespace config
{

// How many threads are used to generate cards and buttons. Must not exceed letter amount 26<
const int num_of_threads = 26;

// Max pair amount: 26 letters times 1000 numbers, from a0 to z999.
const int MAX_PAIRS = 26 * 1000;

const std::string BG_IMAGE_PATH = ":/bg_image.png";
}

#endif // CONFIG_HH
