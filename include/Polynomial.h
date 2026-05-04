#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <vector>
#include <iostream>
#include "GaloisField.h"

class Polynomial{
private:    
    std::vector<uint16_t> coef;       // coef[0] + coef[1]·x + coef[2]·x^2 + ...
    GaloisField gf;
    void trim();

public:
    Polynomial(const GaloisField &galoisField, const std::vector<uint16_t> &coefficients);
    int getDegree() const;
    uint16_t getCoef(int degree) const;
    int setCoef(int degree, uint16_t value);

    Polynomial add(const Polynomial& p2) const;
    Polynomial multiply(const Polynomial& p2) const;

    // Operator overloads delegating to add/multiply
    Polynomial operator+(const Polynomial& p2) const;
    Polynomial operator*(const Polynomial& p2) const;

    uint16_t evaluate(uint16_t x);

    void print();

};
#endif