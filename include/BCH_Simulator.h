#ifndef BCH_SIMULATOR_H
#define BCH_SIMULATOR_H

#include <vector>
#include <string>
#include <iostream>
#include "BCH_Codec.h"
#include "Channel.h"

struct PointResults {
    double ebno_db;
    double ber;
    double ber_uncoded;
    double cwer;
    long codewords_simulated;
    double avg_enc_us; 
    double avg_dec_us;
};

struct SimulationConfig {
    int m = 7;
    int t = 10;
    double ebno_min = 0.0;
    double ebno_max = 12.0;
    double step = 1.0;
    int min_codeword_errors = 100; 
    long max_codewords = 1000000;  
    long long target_total_bits = 1e8;
    bool use_file = false;      
    std::string input_path = "";     
};

class BCH_Simulator {
public:
    BCH_Simulator(const SimulationConfig& config);
    void run();

private:
    SimulationConfig cfg;
    std::vector<PointResults> all_results;

    PointResults simulatePoint(BCH_Codec& bch, Channel& channel, double ebno_db);

    void processFile(BCH_Codec &bch, Channel &channel, double ebno_db, const std::vector<uint16_t> &file_bits, std::vector<uint16_t> &out_corrected, std::vector<uint16_t> &out_noisy);

    // Utilidades
    std::vector<uint16_t> readFileToBits(const std::string& path);
    void saveBitsToFile(const std::vector<uint16_t>& bits, const std::string& path);
    void exportCSV(int n, int k);
    int getDefaultPrimitivePoly(int m);
};

#endif