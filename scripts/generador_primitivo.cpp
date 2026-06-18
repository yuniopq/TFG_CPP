#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <iomanip>

// Función matemática para comprobar primitividad
bool isPrimitive(int m, uint32_t poly) {
    uint32_t n = (1 << m) - 1;
    uint32_t state = 1; 
    
    for (uint32_t i = 1; i <= n; ++i) {
        state <<= 1;
        if (state & (1 << m)) state ^= poly; 
        if (state == 1 && i < n) return false;
    }
    return state == 1;
}

int main() {
    std::cout << "--- Generador de Polinomios Primitivos GF(2^m) ---\n";
    
    std::ofstream archivo("todos_los_polinomios.txt");
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo de salida.\n";
        return 1;
    }

    archivo << "--- Listado Completo de Polinomios Primitivos GF(2^m) ---\n\n";

    for (int m = 3; m <= 15; ++m) {
        std::vector<uint32_t> encontrados;
        
        uint32_t start = (1 << m) + 1;
        uint32_t end = (1 << (m + 1)) - 1;

        for (uint32_t poly = start; poly <= end; poly += 2) {
            if (isPrimitive(m, poly)) {
                encontrados.push_back(poly);
            }
        }
        
        std::cout << "Procesado m=" << m << " (" << encontrados.size() << " polinomios encontrados).\n";

        archivo << "========================================\n";
        archivo << "Grado m = " << m << " | Total encontrados: " << encontrados.size() << "\n";
        archivo << "========================================\n";
        
        for (size_t i = 0; i < encontrados.size(); ++i) {
            archivo << "0x" << std::hex << std::uppercase << encontrados[i] << std::dec;
            
            if (i < encontrados.size() - 1) {
                archivo << ", ";
            }
            
            if ((i + 1) % 10 == 0) archivo << "\n";
        }
        archivo << "\n\n";
    }

    archivo.close();
    std::cout << "\n¡Proceso completado! Todos los resultados se han guardado en 'todos_los_polinomios.txt'.\n";
    
    return 0;
}
