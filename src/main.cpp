#include <iostream>
#include "../include/GaloisField.h"
#include "../include/BCH_Codec.h" 
#include "../include/Polynomial.h"
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

    std::cout << "--- Testing Polynomials ---" << std::endl;

    // Creamos p1(x) = x + 2  (alfa^1 * x + alfa^0)
    // Coeficientes en orden ascendente: [2, 1]
    Polynomial p1(gf, {2, 1}); 

    // Creamos p2(x) = x + 3
    Polynomial p2(gf, {3, 1});

    // Test 1: Suma (x + 2) + (x + 3) = 0x + 1 = 1
    Polynomial p_sum = p1 + p2;
    std::cout << "p1(x) + p2(x) degree: " << p_sum.getDegree() << " (Expected: 0)" << std::endl;
    std::cout << "Result coeff 0: " << p_sum.getCoef(0) << " (Expected: 1)" << std::endl;

    // Test 2: Multiplicación (x + 2)(x + 3) = x^2 + (2+3)x + (2*3) 
    // En GF(256): 2+3 = 1, 2*3 = 6 -> x^2 + x + 6
    Polynomial p_mul = p1 * p2;
    std::cout << "p1(x) * p2(x) degree: " << p_mul.getDegree() << " (Expected: 2)" << std::endl;
    std::cout << "Coeffs: [" << p_mul.getCoef(0) << ", " << p_mul.getCoef(1) << ", " << p_mul.getCoef(2) << "]" << std::endl;
    std::cout << "Expected: [6, 1, 1]" << std::endl;

    // Test 3: Evaluación (Horner)
    // Evaluar p1(x) = x + 2 en x = 5. Resultado: 5 ^ 2 = 7
    int eval = p1.evaluate(5);
    std::cout << "p1(5) = " << eval << " (Expected: 7)" << std::endl;

    std::cout << "\n--- Testing BCH Codec: BCH(15, 7) ---" << std::endl;

    // m=4 (GF(16)), t=2, polinomio primitivo x^4 + x + 1 (0x13)
    int m = 4;
    int t_err = 2;
    int prim_poly = 0x13; 

    try {
        BCH_Codec bch(m, t_err, prim_poly);

        std::cout << "Parameters: n=" << bch.getN() << ", k=" << bch.getK() 
                << ", t=" << bch.getT() << std::endl;

        // Test 1: Verificar el Generador
        // Para BCH(15, 7), el grado del generador debe ser n - k = 8
        bch.printGeneratorPolynomial();
        std::cout << "Expected degree: 8" << std::endl;

        // Test 2: Codificación Sistemática
        // Mensaje de k=7 bits (puedes usar 0s y 1s)
        std::vector<int> msg = {1, 0, 1, 1, 0, 0, 1}; 
        std::vector<int> encoded = bch.encode(msg);

        std::cout << "Message:  ";
        for(int bit : msg) std::cout << bit << " ";
        std::cout << "\nEncoded:  ";
        for(int bit : encoded) std::cout << bit << " ";
        std::cout << std::endl;

        // Verificación de propiedad sistemática: 
        // En tu implementación, el mensaje debería estar en la parte "alta" (los últimos k bits)
        bool systematic_ok = true;
        for(int i = 0; i < bch.getK(); i++) {
            if(encoded[bch.getN() - bch.getK() + i] != msg[i]) {
                systematic_ok = false;
            }
        }
        std::cout << "Systematic Property Check: " << (systematic_ok ? "PASSED ✅" : "FAILED ❌") << std::endl;

        // Test 3: Decodificación (Sin errores por ahora)
        std::vector<int> decoded = bch.decode(encoded);
        bool decode_ok = true;
        for(int i = 0; i < msg.size(); i++) {
            if(decoded[i] != msg[i]) decode_ok = false;
        }
        std::cout << "No-error Decoding Check: " << (decode_ok ? "PASSED ✅" : "FAILED ❌") << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error en BCH: " << e.what() << std::endl;
    }

    return 0;
}