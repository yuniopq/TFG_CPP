#ifndef CHANNEL_H
#define CHANNEL_H

#include <cstdint>
#include <vector>
#include <random>
#include <cmath>

class Channel{
private:
    std::mt19937 rng; // Random number generator

public:
    Channel();

    std::vector<uint16_t> applyBSC(const std::vector<uint16_t>& bits, double bit_error_rate);
    std::vector<double> transmitAWGN(const std::vector<uint16_t>& bits, double es_n0_db);
    std::vector<uint16_t> applyAWGNHardDecision(const std::vector<uint16_t>& bits, double es_n0_db);

};

#endif