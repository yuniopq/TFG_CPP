#include "Channel.h"
#include <algorithm>
#include <stdexcept>

double dbToLinear(double db_value) {
    return std::pow(10.0, db_value / 10.0);
}

Channel::Channel() {
    // Initialize the random number generator with a random seed
    std::random_device rd;
    rng = std::mt19937(rd());
}

std::vector<uint16_t> Channel::applyBSC(const std::vector<uint16_t>& bits, double bit_error_rate) {
    if (bit_error_rate < 0.0 || bit_error_rate > 1.0) {
        throw std::invalid_argument("bit_error_rate must be between 0 and 1");
    }

    std::bernoulli_distribution flip(bit_error_rate);
    std::vector<uint16_t> noisy_bits = bits;

    for (uint16_t& bit : noisy_bits) {
        if (flip(rng)) {
            bit ^= 1;
        }
    }

    return noisy_bits;
}

std::vector<double> Channel::transmitAWGN(const std::vector<uint16_t>& bits, double es_n0_db) {
    const double es_n0_linear = dbToLinear(es_n0_db);
    const double sigma = std::sqrt(1.0 / (2.0 * es_n0_linear));

    std::normal_distribution<double> noise(0.0, sigma);
    std::vector<double> samples;
    samples.reserve(bits.size());

    for (uint16_t bit : bits) {
        const double symbol = (bit == 0) ? 1.0 : -1.0;
        samples.push_back(symbol + noise(rng));
    }

    return samples;
}

std::vector<uint16_t> Channel::applyAWGNHardDecision(const std::vector<uint16_t>& bits, double es_n0_db) {
    std::vector<double> samples = transmitAWGN(bits, es_n0_db);
    std::vector<uint16_t> hard_bits;
    hard_bits.reserve(samples.size());

    for (double sample : samples) {
        hard_bits.push_back(sample >= 0.0 ? 0u : 1u);
    }

    return hard_bits;
}