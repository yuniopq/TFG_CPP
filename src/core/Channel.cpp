#include "Channel.h"

Channel::Channel() {
    // Initialize the random number generator with a random seed
    std::random_device rd;
    rng = std::mt19937(rd());
}