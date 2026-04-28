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

    // GaloisField gf(8, 0x11D);

    // std::cout << "--- Testing Polynomials ---" << std::endl;

    // // Creamos p1(x) = x + 2  (alfa^1 * x + alfa^0)
    // // Coeficientes en orden ascendente: [2, 1]
    // Polynomial p1(gf, {2, 1}); 

    // // Creamos p2(x) = x + 3
    // Polynomial p2(gf, {3, 1});

    // // Test 1: Suma (x + 2) + (x + 3) = 0x + 1 = 1
    // Polynomial p_sum = p1.add(p2);
    // std::cout << "p1(x) + p2(x) degree: " << p_sum.get_degree() << " (Expected: 0)" << std::endl;
    // std::cout << "Result coeff 0: " << p_sum.get_coeff(0) << " (Expected: 1)" << std::endl;

    // // Test 2: Multiplicación (x + 2)(x + 3) = x^2 + (2+3)x + (2*3) 
    // // En GF(256): 2+3 = 1, 2*3 = 6 -> x^2 + x + 6
    // Polynomial p_mul = p1.multiply(p2);
    // std::cout << "p1(x) * p2(x) degree: " << p_mul.get_degree() << " (Expected: 2)" << std::endl;
    // std::cout << "Coeffs: [" << p_mul.get_coeff(0) << ", " << p_mul.get_coeff(1) << ", " << p_mul.get_coeff(2) << "]" << std::endl;
    // std::cout << "Expected: [6, 1, 1]" << std::endl;

    // // Test 3: Evaluación (Horner)
    // // Evaluar p1(x) = x + 2 en x = 5. Resultado: 5 ^ 2 = 7
    // int eval = p1.evaluate(5);
    // std::cout << "p1(5) = " << eval << " (Expected: 7)" << std::endl;

    return 0;
}