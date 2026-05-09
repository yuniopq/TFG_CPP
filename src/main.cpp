#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include "GaloisField.h"
#include "BCH_Codec.h"
#include "Channel.h"
#include "Polynomial.h"

using namespace std;
using namespace std::chrono;

struct SimulationConfig {
    int m = 4;
    int t = 2;
    int num_iter = 100;
    double ber_canal = 0.001;
};

struct SimulationResults {
    double avg_enc = 0;
    double avg_dec = 0;
    double fer = 0;

    long long total_enc = 0;
    long long total_dec = 0;
    int bloques_erroneos = 0;
};

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

        if (argc >= 4) cfg.num_iter = stoi(argv[3]);
        if (argc >= 5) cfg.ber_canal = stod(argv[4]);
    }

    return cfg;
}

void validateParameters(const SimulationConfig& cfg) {
    int n = (1 << cfg.m) - 1;

    if (cfg.m < 1 || cfg.m > 15)
        throw invalid_argument("m debe estar entre 1 y 15");

    if (cfg.t <= 0 || 2LL * cfg.t >= n)
        throw invalid_argument("t debe cumplir 1 <= 2t < n");

    if (cfg.num_iter <= 0)
        throw invalid_argument("num_iter debe ser positivo");

    if (cfg.ber_canal < 0.0 || cfg.ber_canal > 1.0)
        throw invalid_argument("ber_canal debe estar entre 0 y 1");
}

vector<uint16_t> generateRandomMessage(int k) {
    vector<uint16_t> msg(k);

    for (int i = 0; i < k; i++)
        msg[i] = rand() % 2;

    return msg;
}

void printProgress(int iter, int total) {
    if ((iter + 1) % (total / 10 + 1) == 0) {
        cout << "Progreso: "
             << fixed << setprecision(0)
             << (double)(iter + 1) / total * 100
             << "%" << endl;
    }
}

SimulationResults runSimulation( BCH_Codec& bch, Channel& channel, const SimulationConfig& cfg ) {
    SimulationResults results;

    int k = bch.getK();

    for (int iter = 0; iter < cfg.num_iter; iter++) {

        vector<uint16_t> message = generateRandomMessage(k);

        // Codificación
        auto start_e = high_resolution_clock::now();
        vector<uint16_t> encoded = bch.encode(message);
        auto end_e = high_resolution_clock::now();

        results.total_enc += duration_cast<microseconds>(end_e - start_e).count();

        // Canal
        vector<uint16_t> noisy = channel.applyBSC(encoded, cfg.ber_canal);

        // Decodificación
        auto start_d = high_resolution_clock::now();
        vector<uint16_t> decoded = bch.decode(noisy);
        auto end_d = high_resolution_clock::now();

        results.total_dec +=
            duration_cast<microseconds>(end_d - start_d).count();

        // Verificación
        if (message != decoded)
            results.bloques_erroneos++;

        printProgress(iter, cfg.num_iter);
    }

    results.avg_enc =
        (double)results.total_enc / cfg.num_iter;

    results.avg_dec =
        (double)results.total_dec / cfg.num_iter;

    results.fer =
        (double)results.bloques_erroneos / cfg.num_iter;

    return results;
}

void printResults( const SimulationResults& results ) {
    cout << "\n----------------------------------------" << endl;

    cout << "PROMEDIOS (us): "
         << "Enc: " << results.avg_enc
         << " | Dec: " << results.avg_dec << endl;

    cout << "FIABILIDAD: FER: "
         << (results.fer * 100)
         << "% (" << results.bloques_erroneos
         << " fallos)" << endl;

    cout << "----------------------------------------" << endl;
}

void exportCSV( const SimulationConfig& cfg, const SimulationResults& results, int n, int k ) {
    mkdir("results", 0777);
    mkdir("results/csv", 0777);

    string csv_path =
        "results/csv/data_m" + to_string(cfg.m) + ".csv";

    bool existe = ifstream(csv_path).good();

    ofstream outfile(csv_path, ios::app);

    if (!existe) {
        outfile << "m,t,n,k,prob_error,avg_enc_us,avg_dec_us,fer\n";
    }

    outfile << cfg.m << ","
            << cfg.t << ","
            << n << ","
            << k << ","
            << cfg.ber_canal << ","
            << results.avg_enc << ","
            << results.avg_dec << ","
            << results.fer << "\n";

    outfile.close();

    cout << "Datos guardados en: "
         << csv_path << endl;
}

int main(int argc, char* argv[]) {
    try {

        srand(time(nullptr));

        SimulationConfig cfg = parseArguments(argc, argv);

        validateParameters(cfg);

        int prim_poly =
            getDefaultPrimitivePoly(cfg.m);

        if (prim_poly == 0)
            throw invalid_argument("m no soportado");

        int n = (1 << cfg.m) - 1;

        auto start_i = high_resolution_clock::now();

        BCH_Codec bch(cfg.m, cfg.t, prim_poly);
        Channel channel;

        auto end_i = high_resolution_clock::now();

        int k = bch.getK();

        auto dur_init =
            duration_cast<microseconds>( end_i - start_i ).count(); 

        cout << "\n=== SIMULADOR BCH ===" << endl;

        cout << "Configuracion: ("
             << n << ", "
             << k << ") t="
             << cfg.t << endl;

        cout << "Inicializacion: "
             << dur_init
             << " us" << endl;

        cout << "Canal BSC p="
             << cfg.ber_canal
             << " | Iteraciones="
             << cfg.num_iter
             << endl;

        SimulationResults results = runSimulation(bch, channel, cfg);

        printResults(results);

        exportCSV(cfg, results, n, k);

    }
    catch (const exception& e) {
        cerr << "Error critico: " << e.what() << endl;

        return 1;
    }

    return 0;
}