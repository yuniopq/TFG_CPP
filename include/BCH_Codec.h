#ifndef BCH_CODEC_H
#define BCH_CODEC_H

#include <vector>
#include <memory>
#include <unordered_set>
#include "GaloisField.h"
#include "Polynomial.h"

class BCH_Codec {
private:
    int m;                          // Grado del cuerpo GF(2^m)
    int t;                          // Capacidad de corrección de errores
    int n;                          // Longitud del código (2^m - 1)
    int k;                          // Bits de información
    GaloisField gf;                 // Cuerpo de Galois
    Polynomial generator;           // Polinomio generador

    void computeGeneratorPolynomial();

public:
    BCH_Codec(int m, int t, int primitive_poly);
    
    // Codificación: añade bits de paridad al mensaje
    std::vector<uint16_t> encode(const std::vector<uint16_t>& message);

    std::vector<uint16_t> encodeLFSR(const std::vector<uint16_t> &message);

    // Cálculo del síndrome para detección de errores
    std::vector<uint16_t> syndrome(const std::vector<uint16_t>& received, bool &error);

    // Algoritmo de Berlekamp-Massey para el polinomio localizador
    Polynomial berlekampMassey(const std::vector<uint16_t>& syndrome);
    
    // Decodificación: detecta y corrige errores
    bool decode(const std::vector<uint16_t> &received, std::vector<uint16_t> &msg_out);
    
    // Obtención de parámetros
    int getN() const { return n; }
    int getK() const { return k; }
    int getT() const { return t; }
    
    // Depuración
    void printGeneratorPolynomial();
};

#endif