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
        // 10^8 bits is a good trade-off between precision and speed
        long long target_bits = 100000000; 
        cfg.max_codewords = target_bits / n;
        
        // Enforce a minimum number of codewords for very large m
        if (cfg.max_codewords < 100) cfg.max_codewords = 100; 
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