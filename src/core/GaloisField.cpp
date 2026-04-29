#include "../../include/GaloisField.h"

GaloisField::GaloisField(int m, int primitive_poly): m(m), primitive_poly(primitive_poly) {
    size = 1<<m;  // 2^m
    
    log_table.resize(size);
    exp_table.resize(2*size);

    int val=1;
    for (int i = 0; i < size-1; i++){   //size-1 bcos there is no "0"
    
        exp_table[i] = val;
        log_table[val] = i;
        
        val <<= 1;                      // val*=2
        if (val & size)                 // if (val >= size)
            val ^= primitive_poly;      // val %= primitive_poly
    }

    // Optimization: avoid modulo during product
    for (int i=size-1; i<(size-1)*2; i++){
        exp_table[i] = exp_table[i - (size-1)];
    }

    log_table[0] = -1;                  // log(0) doesn't exist

}

int GaloisField::add(int a, int b){
    return a^b;
}

int GaloisField::multiply(int a, int b){
    if (a==0||b==0) return 0;
    return exp_table[log_table[a] + log_table[b]];
}

int GaloisField::divide(int a, int b){
    if (b==0)   __throw_runtime_error("Division by zero");
    if (a==0)   return 0;
    return exp_table[log_table[a] - log_table[b] + (size-1)];
}

int GaloisField::power(int a, int e){
    if (a == 0) return 0;
    if (e == 0) return 1;
    int exp = log_table[a]*e % (size-1);
    return exp_table[exp];
}

int GaloisField::inverse(int a) {
    if (a == 0) __throw_runtime_error("Cannot invert zero");
    return exp_table[(size - 1) - log_table[a]];
}