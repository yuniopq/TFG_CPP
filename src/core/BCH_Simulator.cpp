#include "BCH_Simulator.h"
#include <chrono>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <sys/stat.h>
#include <random>
#include <omp.h>
#include <filesystem>

using namespace std;
using namespace std::chrono;

BCH_Simulator::BCH_Simulator(const SimulationConfig& config) : cfg(config) {}

void BCH_Simulator::run() {
    int prim_poly = getDefaultPrimitivePoly(cfg.m);
    BCH_Codec bch(cfg.m, cfg.t, prim_poly);
    Channel channel;
    int n = bch.getN();
    int k = bch.getK();

    vector<uint16_t> file_bits;
    if (cfg.use_file) {
        cout << "[FILE] Loading file: " << cfg.input_path << endl;
        file_bits = readFileToBits(cfg.input_path);
        cout << "[FILE] Total bits: " << file_bits.size() << " (" << file_bits.size()/k << " blocks)" << endl;
    }

    cout << "\n=== BCH SIMULATION (" << n << "," << k << ") t=" << cfg.t << " ===" << endl;
    cout << "------------------------------------------------------------------------" << endl;
    cout << setw(10) << "Eb/N0(dB)" << setw(15) << "BER" << setw(15) << "BER_Uncoded" << setw(15) << "CWER" << setw(15) << "Codewords" << endl;
    cout << "------------------------------------------------------------------------" << endl;

    for (double e = cfg.ebno_min; e <= cfg.ebno_max; e += cfg.step) {
        vector<uint16_t> corrected_bits;
        PointResults pr = simulatePoint(bch, channel, e, file_bits, corrected_bits);
        all_results.push_back(pr);
        
        cout << setw(10) << fixed << setprecision(1) << e 
             << setw(15) << scientific << setprecision(3) << pr.ber 
             << setw(15) << scientific << setprecision(3) << pr.ber_uncoded 
               << setw(15) << pr.cwer 
               << setw(15) << fixed << (long)pr.codewords_simulated << endl;

        if (cfg.use_file) {
            string out_name = "results/out_EbNo_" + to_string((int)e) + "_" + cfg.input_path;
            saveBitsToFile(corrected_bits, out_name);
        }
        
        if (pr.cwer == 0 && !cfg.use_file) break; 
    }
    exportCSV(n, k);
}

PointResults BCH_Simulator::simulatePoint(BCH_Codec& bch, Channel& channel, double ebno_db, 
                                          const std::vector<uint16_t>& file_bits, 
                                          std::vector<uint16_t>& out_corrected_bits) {
    PointResults res;
    res.ebno_db = ebno_db;
    int k = bch.getK();
    int n = bch.getN();
    double rate = (double)k / n;
    double esno_db = ebno_db + 10.0 * log10(rate); 
    
    long total_bit_errors = 0, total_bit_errors_uncoded = 0, total_codeword_errors = 0, codewords = 0;
    long long total_enc_time = 0, total_dec_time = 0;

    if (cfg.use_file) {
        out_corrected_bits.clear();
        out_corrected_bits.resize(file_bits.size(), 0);
    }
    long max_iterations = cfg.use_file ? (file_bits.size() / k) : cfg.max_codewords;

    #pragma omp parallel
    {
        std::mt19937 rng(std::random_device{}());
        BCH_Codec local_bch = bch;
        Channel local_channel = channel;

        // Metemos casi todos los contadores en la cláusula reduction
        #pragma omp for schedule(dynamic) reduction(+:total_bit_errors, total_bit_errors_uncoded, total_enc_time, total_dec_time, codewords)
        for (long iter = 0; iter < max_iterations; ++iter) {
            
            // Comprobación de parada temprana 
            if (!cfg.use_file) {
                long cur_codeword_errors;
                #pragma omp atomic read
                cur_codeword_errors = total_codeword_errors;
                if (cur_codeword_errors >= cfg.min_codeword_errors) continue;
            }

            vector<uint16_t> message(k);
            if (cfg.use_file) {
                for (int i = 0; i < k; i++) message[i] = file_bits[iter * k + i];
            } else {
                for (int i = 0; i < k; i++) message[i] = (rng() % 2);
            }

            auto t_enc_start = high_resolution_clock::now();
            vector<uint16_t> encoded = local_bch.encode(message);
            auto enc_us = duration_cast<microseconds>(high_resolution_clock::now() - t_enc_start).count();

            vector<uint16_t> noisy = local_channel.applyAWGNHardDecision(encoded, esno_db);

            auto t_dec_start = high_resolution_clock::now();
            vector<uint16_t> decoded;
            bool success = local_bch.decode(noisy, decoded);
            auto dec_us = duration_cast<microseconds>(high_resolution_clock::now() - t_dec_start).count();
            
            if (cfg.use_file) {
                size_t base_index = iter * k;
                for (size_t i = 0; i < decoded.size(); ++i) {
                    out_corrected_bits[base_index + i] = decoded[i];
                }
            }

            vector<uint16_t> noisy_uncoded = local_channel.applyAWGNHardDecision(message, ebno_db);
            int bit_errors_in_codeword = 0;
            int bit_errors_uncoded_in_codeword = 0;
            for (int i = 0; i < k; i++) {
                if (message[i] != noisy_uncoded[i]) bit_errors_uncoded_in_codeword++;
                if (message[i] != decoded[i]) bit_errors_in_codeword++;
            }

            // Actualización de errores
            if (!success || bit_errors_in_codeword > 0) {
                #pragma omp atomic
                total_codeword_errors++;
                
                total_bit_errors += bit_errors_in_codeword;
            }

            total_bit_errors_uncoded += bit_errors_uncoded_in_codeword;
            total_enc_time += enc_us;
            total_dec_time += dec_us;
            codewords++;
        }
    }

    if (codewords > 0) {
        res.ber = (1.0 * total_bit_errors) / (codewords * k);
        res.ber_uncoded = (1.0 * total_bit_errors_uncoded) / (codewords * k);
        res.cwer = (1.0 * total_codeword_errors) / codewords;
        res.avg_enc_us = (1.0 * total_enc_time) / codewords;
        res.avg_dec_us = (1.0 * total_dec_time) / codewords;
    }
    res.codewords_simulated = codewords;
    return res;
}

// Auxiliary methods moved from main into the class
int BCH_Simulator::getDefaultPrimitivePoly(int m) {
    switch (m) {
        case 3:  return 0b1011;
        case 4:  return 0b10011;
        case 5:  return 0b100101;
        case 6:  return 0b1000011;
        case 7:  return 0b10001001;
        case 8:  return 0b100011101;
        case 9:  return 0b1000010001;
        case 10: return 0b10000001001;
        case 11: return 0b100000000101;
        case 12: return 0b1000001010011;
        case 13: return 0b10000000011011;
        case 14: return 0b100010000000011;
        case 15: return 0b1000000000000011;
        default: return 0;
    }
}

void BCH_Simulator::exportCSV(int n, int k) {
    mkdir("results", 0777);
    mkdir("results/csv", 0777);
    string filename = "results/csv/BCH_m" + to_string(cfg.m) + "_t" + to_string(cfg.t) + ".csv";
    ofstream outfile(filename, ios::trunc);
    outfile << "m,t,n,k,ebno_db,ber,ber_uncoded,cwer,codewords,avg_enc_us,avg_dec_us\n";
    for (const auto& r : all_results) {
        outfile << cfg.m << "," << cfg.t << "," << n << "," << k << "," 
                << r.ebno_db << "," << r.ber << "," << r.ber_uncoded << "," << r.cwer << "," 
                << r.codewords_simulated << "," << r.avg_enc_us << "," << r.avg_dec_us << "\n";
    }
    cout << "\n[OK] Datos guardados en: " << filename << endl;
}

std::vector<uint16_t> BCH_Simulator::readFileToBits(const std::string& path) {

    auto size = std::filesystem::file_size(path);

    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Could not open file: " + path);

    std::vector<uint16_t> bits;
    bits.reserve(size * 8);

    char byte;
    while(file.get(byte))
        for(int i = 0; i < 8; i++)
            bits.push_back(byte & (1 << i)? 1 : 0);
    return bits;
}

void BCH_Simulator::saveBitsToFile(const std::vector<uint16_t>& bits, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    for (size_t i = 0; i < bits.size(); i += 8) {
        unsigned char byte = 0;
        for (int j = 0; j < 8 && (i + j) < bits.size(); j++) {
            if (bits[i + j]) byte |= (1 << j);
        }
        file.put(byte);
    }
}