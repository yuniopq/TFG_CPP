#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <chrono>
#include "GaloisField.h"
#include "BCH_Codec.h"
#include "Polynomial.h"

using namespace std;

// Función para obtener el polinomio primitivo estándar según 'm'
int getDefaultPrimitivePoly(int m) {
    switch (m) {
        case 3:  return 0b1011;        // x^3 + x + 1
        case 4:  return 0b10011;       // x^4 + x + 1
        case 5:  return 0b100101;      // x^5 + x^2 + 1
        case 6:  return 0b1000011;     // x^6 + x + 1
        case 7:  return 0b10001001;    // x^7 + x^3 + 1
        case 8:  return 0b100011101;   // x^8 + x^4 + x^3 + x^2 + 1
        case 9:  return 0b1000010001;  // x^9 + x^4 + 1
        case 10: return 0b10000001001; // x^10 + x^3 + 1
        case 11: return 0b100000000101;// x^11 + x^2 + 1
        case 12: return 0b1000001010011;// x^12 + x^6 + x^4 + x + 1
        case 13: return 0b10000000011011;// x^13 + x^4 + x^3 + x + 1
        case 14: return 0b100010000000011;// x^14 + x^10 + x^6 + x + 1
        case 15: return 0b1000000000000011;// x^15 + x + 1
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
        auto start = std::chrono::high_resolution_clock::now();

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

        // El constructor requiere m, t y el polinomio primitivo
        BCH_Codec bch(m, t, prim_poly);

        int n = bch.getN(); // Cambiado de getCodeLength()
        int k = bch.getK(); // Cambiado de getMessageLength()
        cout << "n: " << n << ", k: " << k << ", t: " << bch.getT() << endl;

        // 1. Generar mensaje aleatorio de k bits
        vector<uint16_t> message(k);
        for (int i = 0; i < k; i++) message[i] = rand() % 2;
        printVector("Mensaje original", message);

        // 2. Codificar
        vector<uint16_t> encoded = bch.encode(message);
        printVector("Bloque codificado", encoded);

        // 3. Introducir errores
        vector<uint16_t> noisy = encoded;
        cout << "Inyectando " << t << " errores aleatorios..." << endl;
        for (int i = 0; i < t; i++) {
            int pos = rand() % n;
            noisy[pos] ^= 1; // Flip de bit
            cout << "Error en pos: " << pos << endl;
        }
        printVector("Bloque con ruido  ", noisy);

        // 4. Decodificar
        vector<uint16_t> decoded = bch.decode(noisy);
        printVector("Mensaje recuperado", decoded);

        // 5. Comparar
        bool correcto = (message == decoded);
        if (correcto) {
            cout << "\n✅ EXITO: El mensaje se recuperó íntegramente." << endl;
        } else {
            cout << "\n❌ ERROR: El mensaje decodificado no coincide." << endl;
        }
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        std::cout << "\n⏱ Tiempo de ejecución: "
                << elapsed.count()
                << " segundos\n";

    } catch (const exception& e) {
        cerr << "\nExcepcion: " << e.what() << endl;
        return 1;
    }
    return 0;
}