#include <iostream>
#include "../include/GaloisField.h"

int main() {
    // GF(256) with primitive polynomial x^8 + x^4 + x^3 + x^2 + 1 (0x11D)
    GaloisField gf(8, 0x11D);

    std::cout << "--- Galois Field GF(256) Test ---" << std::endl;
    
    // Test 1: Addition (XOR)
    std::cout << "7 + 3 = " << gf.add(7, 3) << " (Expected: 4)" << std::endl;

    // Test 2: Multiplication
    // In GF(256), 2 * 2 = 4, but what about large numbers?
    int a = 128;
    int b = 2;
    std::cout << "128 * 2 = " << gf.multiply(a, b) << " (Expected: 29 because of reduction)" << std::endl;

    // Test 3: Inverse
    int inv = gf.inverse(10);
    std::cout << "Inv(10) = " << inv << " -> 10 * " << inv << " = " << gf.multiply(10, inv) << std::endl;

    return 0;
}