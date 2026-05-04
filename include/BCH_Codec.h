#ifndef BCH_CODEC_H
#define BCH_CODEC_H

#include <vector>
#include <memory>
#include <unordered_set>
#include "GaloisField.h"
#include "Polynomial.h"

using namespace std;

class BCH_Codec {
private:
    int m;                          // Degree of GF(2^m)
    int t;                          // Error correction capability
    int n;                          // Code length (2^m - 1)
    int k;                          // Information bits
    GaloisField gf;                 // Galois Field
    unique_ptr<Polynomial> generator;  // Generator polynomial

    void computeGeneratorPolynomial();

public:
    BCH_Codec(int m, int t, int primitive_poly);
    
    // Encode: add parity bits to message
    vector<int> encode(const vector<int>& message);

    vector<int> encodeLFSR(const vector<int> &message);

    // Decode: detect and correct errors
    vector<int> decode(const vector<int>& received);
    
    // Get parameters
    int getN() const { return n; }
    int getK() const { return k; }
    int getT() const { return t; }
    
    // Debug
    void printGeneratorPolynomial();
};

#endif
