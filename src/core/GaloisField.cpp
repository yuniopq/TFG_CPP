#include "GaloisField.h"
#include <stdexcept>
#include <cassert>

// Inicialización estática (fuera de la clase)
std::vector<uint16_t> GaloisField::log_table;
std::vector<uint16_t> GaloisField::exp_table;
int GaloisField::initialized_m = -1;

GaloisField::GaloisField(int m, int primitive_poly) 
    : m(m), size(0), primitive_poly(primitive_poly) {
    if (m < 1 || m > 15) {
        throw std::invalid_argument("m debe estar entre 1 y 15");
    }
    if (primitive_poly <= 0) {
        throw std::invalid_argument("el polinomio primitivo debe ser positivo");
    }
    
    size = 1 << m;

    // Solo se computa si es un nuevo cuerpo
    if (initialized_m != m) {
        log_table.assign(size, 0);
        exp_table.assign(2 * size, 0);

        int val = 1;
        for (int i = 0; i < size - 1; i++) {
            exp_table[i] = val;
            log_table[val] = i;
            val <<= 1;
            if (val & size) val ^= primitive_poly;
        }

        // Duplicar la tabla de exponentes para evitar la operación módulo
        for (int i = size - 1; i < (size - 1) * 2; i++) {
            exp_table[i] = exp_table[i - (size - 1)];
        }
        initialized_m = m;
    }
}

uint16_t GaloisField::add(uint16_t a, uint16_t b) const {
    return a ^ b;
}

uint16_t GaloisField::multiply(uint16_t a, uint16_t b) const {
    assert(a < size && b < size && "Elemento del cuerpo fuera de rango");
    if (a == 0 || b == 0) return 0;
    return exp_table[log_table[a] + log_table[b]];
}

uint16_t GaloisField::divide(uint16_t a, uint16_t b) const {
    assert(a < size && b < size && "Elemento del cuerpo fuera de rango");
    assert(b != 0 && "División por cero");
    if (a == 0) return 0;
    return exp_table[log_table[a] - log_table[b] + (size - 1)];
}

uint16_t GaloisField::power(uint16_t a, int e) const {
    assert(a < size && "Elemento del cuerpo fuera de rango");
    assert(e >= 0 && "Exponente negativo no soportado");
    
    if (a == 0) return 0;
    if (e == 0) return 1;
    if (e < 0) {
        throw std::invalid_argument("Exponente negativo no soportado");
    }
    int exp = log_table[a] * e % (size - 1);
    return exp_table[exp];
}

uint16_t GaloisField::inverse(uint16_t a) const {
    assert(a < size && "Elemento del cuerpo fuera de rango");
    assert(a != 0 && "No se puede invertir cero");
    return exp_table[(size - 1) - log_table[a]];
}