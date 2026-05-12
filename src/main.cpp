#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>
#include <sys/stat.h>
#include <random>
#include <omp.h>

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

    bool use_file = false;      
    string input_path = "";     
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
        cfg.max_frames = ((2 << cfg.m) > 1000) ? 10000 : 1000000;
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

PointResults simulatePoint(BCH_Codec& bch, Channel& channel, double ebno_db, const SimulationConfig& cfg, 
                           const std::vector<uint16_t>& file_bits, std::vector<uint16_t>& out_corrected_bits) {
    PointResults res;
    res.ebno_db = ebno_db;
    
    int k = bch.getK();
    int n = bch.getN();
    double rate = (double)k / n;
    double esno_db = ebno_db + 10.0 * log10(rate); 
    
    long total_bit_errors = 0, total_bit_errors_uncoded = 0, total_frame_errors = 0, frames = 0;
    long long total_enc_time = 0, total_dec_time = 0;

    // Reservar espacio para los bits de salida si es modo archivo
    if (cfg.use_file) out_corrected_bits.clear();

    long max_iterations = cfg.use_file ? (file_bits.size() / k) : cfg.max_frames;

    // Paralelizamos iteraciones por frame con OpenMP.
    // Cada iter procesa un bloque independiente; usamos copias locales de BCH y Channel por hilo.
    #pragma omp parallel
    {
        std::mt19937 rng(std::random_device{}());

        BCH_Codec local_bch = bch;
        Channel local_channel = channel;

        #pragma omp for schedule(dynamic)
        for (long iter = 0; iter < max_iterations; ++iter) {
            // Lectura atómica del contador de errores para posible parada temprana
            if (!cfg.use_file) {
                long cur_frame_errors;
                #pragma omp atomic read
                cur_frame_errors = total_frame_errors;
                if (cur_frame_errors >= cfg.min_frame_errors) continue;
            }

            // Fuente de mensajes
            vector<uint16_t> message(k);
            if (cfg.use_file) {
                for (int i = 0; i < k; i++) message[i] = file_bits[iter * k + i];
            } else {
                for (int i = 0; i < k; i++) message[i] = (uint16_t)(rng() % 2);
            }

            // Codificación
            auto t_enc_start = high_resolution_clock::now();
            vector<uint16_t> encoded = local_bch.encode(message);
            auto enc_us = duration_cast<microseconds>(high_resolution_clock::now() - t_enc_start).count();

            // Canal
            vector<uint16_t> noisy = local_channel.applyAWGNHardDecision(encoded, esno_db);

            // Decodificación
            auto t_dec_start = high_resolution_clock::now();
            vector<uint16_t> decoded;
            bool success = local_bch.decode(noisy, decoded);
            auto dec_us = duration_cast<microseconds>(high_resolution_clock::now() - t_dec_start).count();

            // Guardado concurrente (modo archivo) bajo sección crítica
            if (cfg.use_file) {
                #pragma omp critical
                out_corrected_bits.insert(out_corrected_bits.end(), decoded.begin(), decoded.end());
            }

            // Métricas
            vector<uint16_t> noisy_uncoded = local_channel.applyAWGNHardDecision(message, ebno_db);
            int bit_errors_in_frame = 0;
            int bit_errors_uncoded_in_frame = 0;
            for (int i = 0; i < k; i++) {
                if (message[i] != noisy_uncoded[i]) bit_errors_uncoded_in_frame++;
                if (message[i] != decoded[i]) bit_errors_in_frame++;
            }

            if (!success || bit_errors_in_frame > 0) {
                #pragma omp atomic
                total_frame_errors++;
                #pragma omp atomic
                total_bit_errors += bit_errors_in_frame;
            } else {
                // si no hubo errores en el frame, no sumamos a total_bit_errors
            }

            #pragma omp atomic
            total_bit_errors_uncoded += bit_errors_uncoded_in_frame;

            #pragma omp atomic
            total_enc_time += enc_us;
            #pragma omp atomic
            total_dec_time += dec_us;

            #pragma omp atomic
            frames++;
        }
    }

    // Cálculos finales
    long long total_bits = frames * k;
    if (frames > 0) {
        res.ber = (1.0 * total_bit_errors) / total_bits;
        res.ber_uncoded = (1.0 * total_bit_errors_uncoded) / total_bits;
        res.fer = (1.0 * total_frame_errors) / frames;
        res.avg_enc_us = (1.0 * total_enc_time) / frames;
        res.avg_dec_us = (1.0 * total_dec_time) / frames;
    } else {
        res.ber = 0;
        res.ber_uncoded = 0;
        res.fer = 0;
        res.avg_enc_us = 0;
        res.avg_dec_us = 0;
    }
    res.frames_simulated = frames;
    return res;
}
// --- Exportación Actualizada ---
void exportCSV(const SimulationConfig& cfg, const vector<PointResults>& all_results, int n, int k) {
    mkdir("results", 0777);
    mkdir("results/csv", 0777);
    string filename = "results/csv/BCH_m" + to_string(cfg.m) + "_t" + to_string(cfg.t) + ".csv";
    
    ofstream outfile(filename, ios::trunc);
    outfile << "m,t,n,k,ebno_db,ber,ber_uncoded,fer,frames,avg_enc_us,avg_dec_us\n";
    for (const auto& r : all_results) {
        outfile << cfg.m << "," << cfg.t << "," << n << "," << k << "," 
                << r.ebno_db << "," << r.ber << "," << r.ber_uncoded << "," << r.fer << "," 
                << r.frames_simulated << "," << r.avg_enc_us << "," << r.avg_dec_us << "\n";
    }
    outfile.close();
    cout << "\n[OK] Datos guardados en: " << filename << endl;
}

int main(int argc, char* argv[]) {
    try {
        srand(time(nullptr));
        SimulationConfig cfg = parseArguments(argc, argv);
        
        // --- CONFIGURACIÓN PARA MODO ARCHIVO (Opcional) ---
        // Puedes activar esto mediante flags o dejarlo preparado así:
        if (argc >= 7) { 
            cfg.use_file = true;
            cfg.input_path = argv[6]; // ./sim 7 10 0 8 1 imagen.bmp
        }

        validateParameters(cfg);

        int prim_poly = getDefaultPrimitivePoly(cfg.m);
        BCH_Codec bch(cfg.m, cfg.t, prim_poly);
        Channel channel;
        int n = bch.getN();
        int k = bch.getK();

        // 1. Cargar bits si estamos en modo archivo
        vector<uint16_t> file_bits;
        if (cfg.use_file) {
            cout << "[FILE] Cargando archivo: " << cfg.input_path << endl;
            file_bits = readFileToBits(cfg.input_path);
            cout << "[FILE] Bits totales: " << file_bits.size() << " (" << file_bits.size()/k << " bloques)" << endl;
        }

        cout << "\n=== SIMULACION BCH (" << n << "," << k << ") t=" << cfg.t << " ===" << endl;
        cout << "------------------------------------------------------------------------" << endl;
        cout << setw(10) << "Eb/N0(dB)" << setw(15) << "BER" << setw(15) << "BER_Uncoded" << setw(15) << "FER" << setw(12) << "Frames" << endl;
        cout << "------------------------------------------------------------------------" << endl;

        vector<PointResults> all_results;
        for (double e = cfg.ebno_min; e <= cfg.ebno_max; e += cfg.step) {
            
            vector<uint16_t> corrected_bits; // Aquí se guardará el archivo recuperado
            
            // Llamada corregida a simulatePoint
            PointResults pr = simulatePoint(bch, channel, e, cfg, file_bits, corrected_bits);
            all_results.push_back(pr);
            
            cout << setw(10) << fixed << setprecision(1) << e 
                << setw(15) << scientific << setprecision(3) << pr.ber 
                << setw(15) << scientific << setprecision(3) << pr.ber_uncoded 
                << setw(15) << pr.fer 
                << setw(12) << fixed << (long)pr.frames_simulated << endl;
            // 2. Si estamos en modo archivo, guardamos el resultado de CADA Eb/N0
            if (cfg.use_file) {
                string out_name = "results/out_EbNo_" + to_string((int)e) + "_" + cfg.input_path;
                corrected_bits.resize(file_bits.size()); // Ajustar al tamaño original (quitar padding)
                saveBitsToFile(corrected_bits, out_name);
            }
            
            if (pr.fer == 0) break; 
        }

        exportCSV(cfg, all_results, n, k);

    } catch (const exception& e) {
        cerr << "[!] Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}