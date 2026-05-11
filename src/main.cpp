#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>
#include <sys/stat.h>

#include "GaloisField.h"
#include "BCH_Codec.h"
#include "Channel.h"
#include "Polynomial.h"

using namespace std;
using namespace std::chrono;

// --- Estructuras Actualizadas ---
struct SimulationConfig {
    int m = 7;
    int t = 10;
    double ebno_min = 0.0;
    double ebno_max = 12.0;
    double step = 1.0;
    int min_frame_errors = 100; 
    long max_frames = 1000000;  
};

struct PointResults {
    double ebno_db;
    double ber;
    double ber_uncoded;
    double fer;
    long frames_simulated;
    double avg_enc_us; 
    double avg_dec_us;
};

std::vector<uint16_t> readFileToBits(const std::string& path){
    std::ifstream file(path, std::ios::binary);
    std::vector<uint16_t> bits;
    char byte;

    if (!file)
        throw std::runtime_error("Could not open file: " + path);

    while(file.get(byte))
        for(int i = 0; i < 8; i++)
            bits.push_back(byte & (1 << i)? 1 : 0);
        
    
    return bits;
}

void saveBitsToFile(const std::vector<uint16_t>& bits, const std::string& path){
    std::ofstream file(path, std::ios::binary);
    for (size_t i = 0; i < bits.size(); i += 8) {
        unsigned char byte = 0;
        for (int j = 0; j < 8 && (i + j) < bits.size(); j++) {
            if (bits[i + j]) {
                byte |= (1 << j);
            }
        }
        file.put(byte);
    }
}

// --- Funciones auxiliares ---
int getDefaultPrimitivePoly(int m) {
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

SimulationConfig parseArguments(int argc, char* argv[]) {
    SimulationConfig cfg;
    if (argc >= 3) {
        cfg.m = stoi(argv[1]);
        cfg.t = stoi(argv[2]);
        if (argc >= 4) cfg.ebno_min = stod(argv[3]);
        if (argc >= 5) cfg.ebno_max = stod(argv[4]);
        if (argc >= 6) cfg.step = stod(argv[5]);
    }
    return cfg;
}

void validateParameters(const SimulationConfig& cfg) {
    int n = (1 << cfg.m) - 1;
    if (cfg.m < 1 || cfg.m > 15) throw invalid_argument("m entre 1 y 15");
    if (cfg.t <= 0 || 2LL * cfg.t >= n) throw invalid_argument("Capacidad t excedida");
}

vector<uint16_t> generateRandomMessage(int k) {
    vector<uint16_t> msg(k);
    for (int i = 0; i < k; i++) msg[i] = rand() % 2;
    return msg;
}

// --- Motor de Simulación ---
PointResults simulatePoint(BCH_Codec& bch, Channel& channel, double ebno_db, const SimulationConfig& cfg) {
    PointResults res;
    res.ebno_db = ebno_db;
    
    int k = bch.getK();
    int n = bch.getN();
    double rate = (double)k / n;
    double esno_db = ebno_db + 10.0 * log10(rate); 
    
    long total_bit_errors = 0;
    long total_bit_errors_uncoded = 0;
    long total_frame_errors = 0;
    long frames = 0;
    long long total_enc_time = 0; // <--- Acumulador codificación
    long long total_dec_time = 0; // <--- Acumulador decodificación

    while (total_frame_errors < cfg.min_frame_errors && frames < cfg.max_frames) {
        vector<uint16_t> message = generateRandomMessage(k);

        // --- CAMINO A: Con BCH ---
        
        // --- Cronometrar Codificación ---
        auto start_enc = high_resolution_clock::now();
        vector<uint16_t> encoded = bch.encode(message);
        auto end_enc = high_resolution_clock::now();
        total_enc_time += duration_cast<microseconds>(end_enc - start_enc).count();

        // Canal
        vector<uint16_t> noisy = channel.applyAWGNHardDecision(encoded, esno_db);

        // --- Cronometrar Decodificación ---
        vector<uint16_t> decoded;
        auto start_dec = high_resolution_clock::now();
        bool success = bch.decode(noisy, decoded);
        auto end_dec = high_resolution_clock::now();
        total_dec_time += duration_cast<microseconds>(end_dec - start_dec).count();

        // Conteo de errores
        int bit_errors_in_frame = 0;
        for (int i = 0; i < k; i++) {
            if (message[i] != decoded[i]) bit_errors_in_frame++;
        }

        // --- CAMINO B: Sin codificar ---
        vector<uint16_t> noisy_uncoded = channel.applyAWGNHardDecision(message, ebno_db);
        
        // Conteo de errores Uncoded
        for (int i = 0; i < k; i++) {
            if (message[i] != noisy_uncoded[i]) total_bit_errors_uncoded++;
        }

        if (!success || bit_errors_in_frame > 0) {
            total_frame_errors++;
            total_bit_errors += bit_errors_in_frame;
        }
        frames++;
    }

    res.ber = (double)total_bit_errors / (frames * k);
    res.ber_uncoded = (double)total_bit_errors_uncoded / (frames * k);
    res.fer = (double)total_frame_errors / frames;
    res.frames_simulated = frames;
    res.avg_enc_us = (double)total_enc_time / frames; // <--- Media codif
    res.avg_dec_us = (double)total_dec_time / frames; // <--- Media decodif
    return res;
}

// --- Exportación Actualizada ---
void exportCSV(const SimulationConfig& cfg, const vector<PointResults>& all_results, int n, int k) {
    mkdir("results", 0777);
    mkdir("results/csv", 0777);
    string filename = "results/csv/BCH_m" + to_string(cfg.m) + "_t" + to_string(cfg.t) + ".csv";
    
    ofstream outfile(filename, ios::trunc);
    // Añadimos avg_enc_us a la cabecera
    outfile << "m,t,n,k,ebno_db,ber,fer,frames,avg_enc_us,avg_dec_us\n";
    for (const auto& r : all_results) {
        outfile << cfg.m << "," << cfg.t << "," << n << "," << k << "," 
                << r.ebno_db << "," << r.ber << "," << r.fer << "," 
                << r.frames_simulated << "," << r.avg_enc_us << "," << r.avg_dec_us << "\n";
    }
    outfile.close();
    cout << "\n[OK] Datos guardados en: " << filename << endl;
}

int main(int argc, char* argv[]) {
    try {
        srand(time(nullptr));
        SimulationConfig cfg = parseArguments(argc, argv);
        validateParameters(cfg);

        int prim_poly = getDefaultPrimitivePoly(cfg.m);
        BCH_Codec bch(cfg.m, cfg.t, prim_poly);
        Channel channel;
        int n = bch.getN();
        int k = bch.getK();

        cout << "\n=== SIMULACION BCH (" << n << "," << k << ") t=" << cfg.t << " ===" << endl;
        cout << "------------------------------------------------------------------------" << endl;
        cout << setw(10) << "Eb/N0(dB)" << setw(15) << "BER" << setw(15) << "FER" << setw(12) << "Frames" << endl;
        cout << "------------------------------------------------------------------------" << endl;

        vector<PointResults> all_results;
        for (double e = cfg.ebno_min; e <= cfg.ebno_max; e += cfg.step) {
            PointResults pr = simulatePoint(bch, channel, e, cfg);
            all_results.push_back(pr);
            
            cout << setw(10) << fixed << setprecision(1) << e 
                 << setw(15) << scientific << setprecision(3) << pr.ber 
                 << setw(15) << pr.fer 
                 << setw(12) << fixed << pr.frames_simulated << endl;
            
            if (pr.fer == 0 && e > 5.0) break;
        }

        exportCSV(cfg, all_results, n, k);

    } catch (const exception& e) {
        cerr << "[!] Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}