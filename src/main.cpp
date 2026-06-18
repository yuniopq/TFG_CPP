#include <iostream>
#include <string>
#include "BCH_Simulator.h"

using namespace std;

SimulationConfig parseArguments(int argc, char* argv[]) {
    SimulationConfig cfg;
    
    // Bucle para leer pares de argumentos (ej: "-m 7")
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        
        if (arg == "-m" && i + 1 < argc) cfg.m = stoi(argv[++i]);
        else if (arg == "-t" && i + 1 < argc) cfg.t = stoi(argv[++i]);
        else if (arg == "-min" && i + 1 < argc) cfg.ebno_min = stod(argv[++i]);
        else if (arg == "-max" && i + 1 < argc) cfg.ebno_max = stod(argv[++i]);
        else if (arg == "-step" && i + 1 < argc) cfg.step = stod(argv[++i]);
        else if (arg == "-bits" && i + 1 < argc) cfg.target_bits = stoll(argv[++i]);
        else if (arg == "-file" && i + 1 < argc) {
            cfg.use_file = true;
            cfg.input_path = argv[++i];
        }
        else if (arg == "-q") {
            cfg.quiet = true;
        }
        else if (arg == "-chan" && i + 1 < argc) {
            cfg.channel_type = argv[++i];
        }
        else if (arg == "-prob" && i + 1 < argc) {
            cfg.bsc_prob = stod(argv[++i]);
        }
        
    }

    if (!cfg.use_file && cfg.channel_type == "bsc" && !cfg.quiet) {
        cout << "[!] Aviso: El canal BSC se reserva para el procesamiento de archivos. Se forzará el uso de AWGN para el análisis Montecarlo." << endl;
    }
    
    int n = (1 << cfg.m) - 1;
    
    // Si es simulación estadística, calculamos el límite de palabras
    if (!cfg.use_file) {
        cfg.max_codewords = cfg.target_bits / n;
        
        // Límites de seguridad
        if (cfg.max_codewords < 50000) cfg.max_codewords = 50000; 
        if (cfg.max_codewords > 2000000) cfg.max_codewords = 2000000;
    }
    
    return cfg;
}

int main(int argc, char* argv[]) {
    try {
        srand(time(nullptr));

        SimulationConfig cfg = parseArguments(argc, argv);

        int n_check = (1 << cfg.m) - 1;
        if (cfg.m < 1 || cfg.m > 15) throw invalid_argument("m debe estar entre 1 y 15");
        if (cfg.t <= 0 || 2LL * cfg.t >= n_check) throw invalid_argument("La capacidad de corrección t excede el límite del bloque");

        BCH_Simulator simulator(cfg);
        simulator.run();

    } catch (const exception& e) {
        cerr << "[!] Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}