#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <chrono>
#include <iomanip> // Para formatear decimales
#include "GaloisField.h"
#include "BCH_Codec.h"
#include "Polynomial.h"

using namespace std;
using namespace std::chrono;

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

void printVector(const string& label, const vector<uint16_t>& vec) {
    cout << label << " (" << vec.size() << " bits): [ ";
    for (size_t i = 0; i < vec.size(); i++) {
        cout << vec[i] << " ";
        if (i == 29 && vec.size() > 30) {
            cout << "...";
            break;
        }
    }
    cout << "]" << endl;
}

int main(int argc, char* argv[]) {
    try {
        srand(time(nullptr));

        int m = 4;
        int t = 2;

        if (argc >= 3) {
            m = stoi(argv[1]);
            t = stoi(argv[2]);
        }

        int prim_poly = getDefaultPrimitivePoly(m);
        if (prim_poly == 0) throw invalid_argument("Polinomio primitivo no definido para m");

        cout << "=== TEST BCH CODEC (" << ((1 << m) - 1) << ", k) ===" << endl;

        // --- MEDICIÓN: INICIALIZACIÓN ---
        auto t_init_start = high_resolution_clock::now();
        
        BCH_Codec bch(m, t, prim_poly);
        
        auto t_init_end = high_resolution_clock::now();
        // --------------------------------

        int n = bch.getN();
        int k = bch.getK();
        cout << "n: " << n << ", k: " << k << ", t: " << bch.getT() << endl;

        // 1. Generar mensaje aleatorio
        vector<uint16_t> message(k);
        for (int i = 0; i < k; i++) message[i] = rand() % 2;
        printVector("Mensaje original", message);

        // --- MEDICIÓN: CODIFICACIÓN ---
        auto t_enc_start = high_resolution_clock::now();
        
        vector<uint16_t> encoded = bch.encode(message);
        
        auto t_enc_end = high_resolution_clock::now();
        // ------------------------------

        // 3. Introducir errores
        vector<uint16_t> noisy = encoded;
        cout << "\nInyectando " << t << " errores aleatorios..." << endl;
        for (int i = 0; i < t; i++) {
            int pos = rand() % n;
            noisy[pos] ^= 1;
        }

        // --- MEDICIÓN: DECODIFICACIÓN ---
        auto t_dec_start = high_resolution_clock::now();
        
        vector<uint16_t> decoded = bch.decode(noisy);
        
        auto t_dec_end = high_resolution_clock::now();
        // --------------------------------

        // 5. Comparar y Resultados
        bool correcto = (message == decoded);
        
        // Calcular duraciones en microsegundos para mayor precisión en procesos rápidos
        auto d_init = duration_cast<microseconds>(t_init_end - t_init_start).count();
        auto d_enc  = duration_cast<microseconds>(t_enc_end - t_enc_start).count();
        auto d_dec  = duration_cast<microseconds>(t_dec_end - t_dec_start).count();

        cout << "\n" << string(40, '-') << endl;
        cout << "REPORTE DE TIEMPOS" << endl;
        cout << string(40, '-') << endl;
        cout << fixed << setprecision(3);
        cout << "1. Inicialización (GF/BCH): " << d_init << " µs (" << d_init / 1000.0 << " ms)" << endl;
        cout << "2. Codificación:           " << d_enc  << " µs (" << d_enc  / 1000.0 << " ms)" << endl;
        cout << "3. Decodificación:         " << d_dec  << " µs (" << d_dec  / 1000.0 << " ms)" << endl;
        cout << "TOTAL PROCESO:             " << (d_init + d_enc + d_dec) / 1000.0 << " ms" << endl;
        cout << string(40, '-') << endl;

        if (correcto) {
            cout << "✅ EXITO: El mensaje se recuperó íntegramente." << endl;
        } else {
            cout << "❌ ERROR: El mensaje decodificado no coincide." << endl;
        }

    } catch (const exception& e) {
        cerr << "\nExcepcion: " << e.what() << endl;
        return 1;
    }
    return 0;
}