#ifndef BCH_CODEC_H
#define BCH_CODEC_H

#include <vector>
#include <memory>
#include <unordered_set>
#include "GaloisField.h"
#include "Polynomial.h"

class BCH_Codec {
private:
    int m;                          // Degree of GF(2^m)
    int t;                          // Error correction capability
    int n;                          // Code length (2^m - 1)
    int k;                          // Information bits
    GaloisField gf;                 // Galois Field
    Polynomial generator;  // Generator polynomial

    void computeGeneratorPolynomial();

public:
    BCH_Codec(int m, int t, int primitive_poly);
    
    // Encode: add parity bits to message
    std::vector<uint16_t> encode(const std::vector<uint16_t>& message);

    std::vector<uint16_t> encodeLFSR(const std::vector<uint16_t> &message);

    std::vector<uint16_t> encodeHorner(const std::vector<uint16_t> &message);

    std::vector<uint16_t> syndrome(const std::vector<uint16_t>& received, bool &error);

    Polynomial belerkampMassey(const std::vector<uint16_t> syndrome);
    // Decode: detect and correct errors
    std::vector<uint16_t> decode(const std::vector<uint16_t>& received);
    
    // Get parameters
    int getN() const { return n; }
    int getK() const { return k; }
    int getT() const { return t; }
    
    // Debug
    void printGeneratorPolynomial();
};

#endif
