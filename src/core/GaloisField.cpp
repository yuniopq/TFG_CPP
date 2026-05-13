#include "GaloisField.h"
#include <stdexcept>
#include <cassert>

// Static initialization (outside the class)
std::vector<uint16_t> GaloisField::log_table;
std::vector<uint16_t> GaloisField::exp_table;
int GaloisField::initialized_m = -1;

GaloisField::GaloisField(int m, int primitive_poly) 
    : m(m), size(0), primitive_poly(primitive_poly) {
    if (m < 1 || m > 15) {
        throw std::invalid_argument("m must be between 1 and 15");
    }
    if (primitive_poly <= 0) {
        throw std::invalid_argument("primitive polynomial must be positive");
    }
    
    size = 1 << m;

    // Only compute this if it is a new field
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
        initialized_m = m;
    }
}

uint16_t GaloisField::add(uint16_t a, uint16_t b) const{
    return a^b;
}

uint16_t GaloisField::multiply(uint16_t a, uint16_t b) const{
    // if ( a >= size || b >= size) {
    //     throw std::out_of_range("Field element out of range");
    // }
    assert(a < size && b < size && "Field element out of range");
    if (a==0||b==0) return 0;
    return exp_table[log_table[a] + log_table[b]];
}

uint16_t GaloisField::divide(uint16_t a, uint16_t b) const{
    // if (a >= size || b >= size) {
    //     throw std::out_of_range("Field element out of range");
    // }
    // if (b==0)   throw std::runtime_error("Division by zero");
    assert(a < size && b < size && "Field element out of range");
    assert(b != 0 && "Division by zero");
    if (a==0)   return 0;
    return exp_table[log_table[a] - log_table[b] + (size-1)];
}

uint16_t GaloisField::power(uint16_t a, int e) const{
    // if (a >= size) {
    //     throw std::out_of_range("Field element out of range");
    // }
    assert(a < size && "Field element out of range");
    assert(e >= 0 && "Negative exponent not supported");
    if (a == 0) return 0;
    if (e == 0) return 1;
    if (e < 0) {
        throw std::invalid_argument("Negative exponent not supported");
    }
    int exp = log_table[a]*e % (size-1);
    return exp_table[exp];
}

uint16_t GaloisField::inverse(uint16_t a) const {
    // if (a >= size) {
    //     throw std::out_of_range("Field element out of range");
    // }
    // if (a == 0) throw std::runtime_error("Cannot invert zero");
    assert(a < size && "Field element out of range");
    assert(a != 0 && "Cannot invert zero");
    return exp_table[(size - 1) - log_table[a]];
}