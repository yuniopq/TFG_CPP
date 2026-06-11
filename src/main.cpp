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
    
    // Si no estamos en modo fichero, se calcula dinámicamente el número de palabras código
    if (!cfg.use_file) {
        long long target_bits = 2000000000LL;   // Volumen objetivo: 2.000 millones de bits
        cfg.max_codewords = target_bits / n;
        
        // Acotación inferior para garantizar significancia estadística en códigos cortos
        if (cfg.max_codewords < 50000) cfg.max_codewords = 50000; 
        
        // Acotación superior para evitar tiempos de cómputo excesivos en configuraciones exigentes
        if (cfg.max_codewords > 2000000) cfg.max_codewords = 2000000;
    }
    
    return cfg;
}

int main(int argc, char* argv[]) {
    try {
        // Inicialización de la semilla para generadores pseudoaleatorios globales
        srand(time(nullptr));

        // 1. Lectura de parámetros de configuración
        SimulationConfig cfg = parseArguments(argc, argv);

        // 2. Validación de restricciones estructurales del código BCH
        int n_check = (1 << cfg.m) - 1;
        if (cfg.m < 1 || cfg.m > 15) throw invalid_argument("m debe estar entre 1 y 15");
        if (cfg.t <= 0 || 2LL * cfg.t >= n_check) throw invalid_argument("La capacidad de corrección t excede el límite del bloque");

        // 3. Inicialización y ejecución del simulador
        BCH_Simulator simulator(cfg);
        simulator.run();

    } catch (const exception& e) {
        cerr << "[!] Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}