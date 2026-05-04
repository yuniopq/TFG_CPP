#include <iostream>
#include "../include/GaloisField.h"
#include "../include/BCH_Codec.h" 
#include "../include/Polynomial.h"
using namespace std;
// Función para imprimir vectores sin saturar la pantalla si son muy grandes
void printVector(const string& label, const vector<int>& vec) {
    cout << label << " (" << vec.size() << " bits): [ ";
    int limit = min((int)vec.size(), 30); // Solo imprime los primeros 30 bits para no llenar la consola
    for (int i = 0; i < limit; i++) {
        cout << vec[i] << " ";
    }
    if (vec.size() > 30) cout << "...";
    cout << "]" << endl;
}

// Diccionario de polinomios primitivos estándar para cada 'm'
int getDefaultPrimitivePoly(int m) {
    switch (m) {
        case 3: return 11;  // x^3 + x + 1
        case 4: return 19;  // x^4 + x + 1
        case 5: return 37;  // x^5 + x^2 + 1
        case 6: return 67;  // x^6 + x + 1
        case 7: return 137; // x^7 + x^3 + 1
        case 8: return 285; // x^8 + x^4 + x^3 + x^2 + 1
        case 9: return 529; // x^9 + x^4 + 1
        case 10: return 1033; // x^10 + x^3 + 1
        default: 
            throw invalid_argument("No tengo un polinomio predefinido para esa 'm'.");
    }
}

int main(int argc, char* argv[]) {
    try {
        // Valores por defecto 
        int m = 4;
        int t = 2;

        // Si el usuario pasa argumentos por terminal (ej: ./bch_test 5 3)
        if (argc >= 3) {
            m = atoi(argv[1]);
            t = atoi(argv[2]);
        }

        int primitive_poly = getDefaultPrimitivePoly(m);

        cout << "========================================" << endl;
        cout << "   INICIALIZANDO CODEC BCH(" << ((1<<m)-1) << ", k)   " << endl;
        cout << "========================================" << endl;
        
        BCH_Codec bch(m, t, primitive_poly);
        
        int n = bch.getN();
        int k = bch.getK();
        int paridad_bits = n - k;

        cout << "Parametros:" << endl;
        cout << "- m: " << m << " (Polinomio primitivo: " << primitive_poly << ")" << endl;
        cout << "- Longitud total (n): " << n << " bits" << endl;
        cout << "- Bits de mensaje (k): " << k << " bits" << endl;
        cout << "- Bits de paridad: " << paridad_bits << " bits" << endl;
        cout << "- Capacidad de correccion (t): " << bch.getT() << " errores" << endl;
        bch.printGeneratorPolynomial();
        cout << "========================================" << endl;

        // Generamos un mensaje dinámico exactamente del tamaño 'k'
        vector<int> message(k);
        for(int i = 0; i < k; i++) {
            message[i] = rand() % 2; // Rellena con 0s y 1s aleatorios
        }
        
        cout << "\n--- FASE DE CODIFICACION ---" << endl;
        printVector("Mensaje original", message);

        vector<int> encoded = bch.encode(message);
        printVector("Mensaje Transmitido", encoded);
        
        // Comprobación de que es sistemático
        bool is_systematic = true;
        for(int i = 0; i < k; i++) {
            if(encoded[paridad_bits + i] != message[i]) {
                is_systematic = false;
                break;
            }
        }

        if (is_systematic) {
            cout << "\n[OK] El codigo es sistematico (el mensaje esta intacto al final del bloque)." << endl;
        } else {
            cout << "\n[ERROR] El mensaje original no coincide con el final del bloque codificado." << endl;
        }

    } catch (const exception& e) {
        cerr << "\n[ERROR FATAL]: " << e.what() << endl;
        cerr << "Asegurate de que 't' no sea tan grande que haga k <= 0." << endl;
    }

    return 0;
}