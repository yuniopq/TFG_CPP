#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <sys/stat.h> // Para crear directorios en Linux/Unix

#include "GaloisField.h"
#include "BCH_Codec.h"
#include "Polynomial.h"

using namespace std;
using namespace std::chrono;

// Función para obtener el polinomio primitivo estándar
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

int main(int argc, char* argv[]) {
    try {
        srand(time(nullptr));

        // 1. Configuración de parámetros (vía argumentos o por defecto)
        int m = 4;
        int t = 2;
        int num_iter = 100;      // Iteraciones por defecto
        double ber_canal = 0.001; // Probabilidad de error de bit (Canal BSC)

        if (argc >= 3) {
            m = stoi(argv[1]);
            t = stoi(argv[2]);
            if (argc >= 4) num_iter = stoi(argv[3]);
            if (argc >= 5) ber_canal = stod(argv[4]);
        }

        if (m < 1 || m > 15) {
            throw invalid_argument("m debe estar entre 1 y 15");
        }

        int n = (1 << m) - 1;
        if (t <= 0 || 2LL * t >= n) {
            throw invalid_argument("t debe cumplir 1 <= 2t < n");
        }

        if (num_iter <= 0) {
            throw invalid_argument("num_iter debe ser positivo");
        }

        if (ber_canal < 0.0 || ber_canal > 1.0) {
            throw invalid_argument("ber_canal debe estar entre 0 y 1");
        }

        int prim_poly = getDefaultPrimitivePoly(m);
        if (prim_poly == 0) throw invalid_argument("m no soportado.");

        // 2. Inicialización del Codec (Medición única)
        auto start_i = high_resolution_clock::now();
        BCH_Codec bch(m, t, prim_poly);
        auto end_i = high_resolution_clock::now();
        
        int k = bch.getK();
        auto dur_init = duration_cast<microseconds>(end_i - start_i).count();

        cout << "\n=== SIMULADOR BCH: ANALISIS DE RENDIMIENTO ===" << endl;
        cout << "Configuracion: (" << n << ", " << k << ") t=" << t << endl;
        cout << "Inicializacion del codec: " << dur_init << " us" << endl;
        cout << "Canal BSC (p=" << ber_canal << ") | Iteraciones: " << num_iter << endl;

        long long total_enc = 0, total_dec = 0;
        int bloques_erroneos = 0;

        // 3. Bucle de Simulación
        for (int iter = 0; iter < num_iter; iter++) {
            // A. Generar mensaje aleatorio
            vector<uint16_t> message(k);
            for (int i = 0; i < k; i++) message[i] = rand() % 2;

            // B. Codificación
            auto start_e = high_resolution_clock::now();
            vector<uint16_t> encoded = bch.encode(message);
            auto end_e = high_resolution_clock::now();
            total_enc += duration_cast<microseconds>(end_e - start_e).count();

            // C. Canal con Ruido (Inyección probabilística)
            vector<uint16_t> noisy = encoded;
            for (int i = 0; i < n; i++) {
                if (((double)rand() / RAND_MAX) < ber_canal) {
                    noisy[i] ^= 1;
                }
            }

            // D. Decodificación
            auto start_d = high_resolution_clock::now();
            vector<uint16_t> decoded = bch.decode(noisy);
            auto end_d = high_resolution_clock::now();
            total_dec += duration_cast<microseconds>(end_d - start_d).count();

            // E. Verificación de éxito
            if (message != decoded) {
                bloques_erroneos++;
            }

            // Barra de progreso simple
            if ((iter + 1) % (num_iter / 10 + 1) == 0) {
                cout << "Progreso: " << fixed << setprecision(0) << (double)(iter) / num_iter * 100 << "%" << endl;
            }
        }

        // 4. Cálculo de métricas
        double avg_enc = (double)total_enc / num_iter;
        double avg_dec = (double)total_dec / num_iter;
        double fer = (double)bloques_erroneos / num_iter;

        // 5. Mostrar resultados por pantalla
        cout << "\n----------------------------------------" << endl;
        cout << "PROMEDIOS (us): Enc: " << avg_enc << " | Dec: " << avg_dec << endl;
        cout << "FIABILIDAD: FER: " << (fer * 100) << "% (" << bloques_erroneos << " fallos)" << endl;
        cout << "----------------------------------------" << endl;

        // 6. Exportación a CSV para el plotter.py
        // Creamos la carpeta 'results' si no existe
        mkdir("results", 0777); 
        mkdir("results/csv", 0777);
        
        string csv_path = "results/csv/data_m" + to_string(m) + ".csv";
        bool existe = ifstream(csv_path).good();
        ofstream outfile(csv_path, ios::app);

        if (!existe) {
            outfile << "m,t,n,k,prob_error,avg_enc_us,avg_dec_us,fer\n";
        }

        outfile << m << "," << t << "," << n << "," << k << ","
                << ber_canal << "," << avg_enc << "," << avg_dec << "," << fer << "\n";
        
        outfile.close();
        cout << "Datos guardados en: " << csv_path << endl;

    } catch (const exception& e) {
        cerr << "Error critico: " << e.what() << endl;
        return 1;
    }
    return 0;
}