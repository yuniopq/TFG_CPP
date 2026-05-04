#include "../../include/GaloisField.h"
#include <stdexcept>

// Inicialización de estáticos (fuera de la clase)
std::vector<int> GaloisField::log_table;
std::vector<int> GaloisField::exp_table;
int GaloisField::initialized_m = -1;

GaloisField::GaloisField(int m, int primitive_poly) 
    : m(m), primitive_poly(primitive_poly) {
    
    size = 1 << m;

    // Solo calculamos si es un campo nuevo
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

        for (int i = size - 1; i < (size - 1) * 2; i++) {
            exp_table[i] = exp_table[i - (size - 1)];
        }
        log_table[0] = -1;
        initialized_m = m;
    }
}

int GaloisField::add(int a, int b) const{
    return a^b;
}

int GaloisField::multiply(int a, int b) const{
    if (a < 0 || a >= size || b < 0 || b >= size) {
        throw std::out_of_range("Field element out of range");
    }
    if (a==0||b==0) return 0;
    return exp_table[log_table[a] + log_table[b]];
}

int GaloisField::divide(int a, int b) const{
    if (a < 0 || a >= size || b < 0 || b >= size) {
        throw std::out_of_range("Field element out of range");
    }
    if (b==0)   throw std::runtime_error("Division by zero");
    if (a==0)   return 0;
    return exp_table[log_table[a] - log_table[b] + (size-1)];
}

int GaloisField::power(int a, int e) const{
    if (a < 0 || a >= size) {
        throw std::out_of_range("Field element out of range");
    }
    if (a == 0) return 0;
    if (e == 0) return 1;
    if (e < 0) {
        throw std::invalid_argument("Negative exponent not supported");
    }
    int exp = log_table[a]*e % (size-1);
    return exp_table[exp];
}

int GaloisField::inverse(int a) const {
    if (a < 0 || a >= size) {
        throw std::out_of_range("Field element out of range");
    }
    if (a == 0) throw std::runtime_error("Cannot invert zero");
    return exp_table[(size - 1) - log_table[a]];
}