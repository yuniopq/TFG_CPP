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
    // If we are not in file mode, estimate the number of codewords needed
    if (!cfg.use_file) {
        long long target_bits = 2000000000LL;   // ← Cambia a 2.000 millones de bits
        cfg.max_codewords = target_bits / n;
        
        // Mínimo para no quedar con muy pocos datos
        if (cfg.max_codewords < 50000) cfg.max_codewords = 50000; 
        
        // Tope superior para no eternizarse en m=15 + t alto
        if (cfg.max_codewords > 2000000) cfg.max_codewords = 2000000;
    }
    return cfg;
}

int main(int argc, char* argv[]) {
    try {
        // Seed for code outside the class RNGs
        srand(time(nullptr));

        // 1. Parse configuration
        SimulationConfig cfg = parseArguments(argc, argv);

        // 2. Validate basic constraints
        int n_check = (1 << cfg.m) - 1;
        if (cfg.m < 1 || cfg.m > 15) throw invalid_argument("m must be between 1 and 15");
        if (cfg.t <= 0 || 2LL * cfg.t >= n_check) throw invalid_argument("t capacity exceeded");

        // 3. Launch simulator
        BCH_Simulator simulator(cfg);
        simulator.run();

    } catch (const exception& e) {
        cerr << "[!] Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}