#include <iostream>
#include "BCH_Simulator.h"

using namespace std;

SimulationConfig parseArguments(int argc, char* argv[]) {
    SimulationConfig cfg;
    if (argc >= 3) {
        cfg.m = stoi(argv[1]);
        cfg.t = stoi(argv[2]);
        if (argc >= 4) cfg.ebno_min = stod(argv[3]);
        if (argc >= 5) cfg.ebno_max = stod(argv[4]);
        if (argc >= 6) cfg.step = stod(argv[5]);
        if (argc >= 7) { 
            cfg.use_file = true;
            cfg.input_path = argv[6];
        }
    }
    int n = (1 << cfg.m) - 1;
    // Si no estamos en modo archivo, calculamos los codewords necesarios
    if (!cfg.use_file) {
        // 10^8 bits es un buen compromiso entre precisión y velocidad
        long long target_bits = 100000000; 
        cfg.max_codewords = target_bits / n;
        
        // Aseguramos un mínimo de codewords para m muy grandes
        if (cfg.max_codewords < 100) cfg.max_codewords = 100; 
    }
    return cfg;
}

int main(int argc, char* argv[]) {
    try {
        // Semilla para cosas que no usen el motor de la clase
        srand(time(nullptr));

        // 1. Obtener configuración
        SimulationConfig cfg = parseArguments(argc, argv);

        // 2. Validar (Lógica básica)
        int n_check = (1 << cfg.m) - 1;
        if (cfg.m < 1 || cfg.m > 15) throw invalid_argument("m debe estar entre 1 y 15");
        if (cfg.t <= 0 || 2LL * cfg.t >= n_check) throw invalid_argument("Capacidad t excedida");

        // 3. Lanzar simulador
        BCH_Simulator simulator(cfg);
        simulator.run();

    } catch (const exception& e) {
        cerr << "[!] Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}