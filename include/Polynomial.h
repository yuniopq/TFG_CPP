#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <vector>
#include <iostream>
#include "GaloisField.h"

class Polynomial {
private:    
    std::vector<uint16_t> coef;       // Coeficientes: coef[0] + coef[1]*x + coef[2]*x^2 + ...
    GaloisField gf;                   // Referencia al cuerpo de Galois subyacente
    
    void trim();                      // Elimina coeficientes nulos de mayor grado

public:
    Polynomial(const GaloisField &galoisField, const std::vector<uint16_t> &coefficients);
    
    // Obtención
    int getDegree() const;
    uint16_t getCoef(int degree) const;

    // Operaciones aritméticas polinómicas
    Polynomial add(const Polynomial& p2) const;
    Polynomial multiply(const Polynomial& p2) const;

    // Sobrecarga de operadores
    Polynomial operator+(const Polynomial& p2) const;
    Polynomial operator*(const Polynomial& p2) const;

    // Evaluación polinómica (Método de Horner)
    uint16_t evaluate(uint16_t x);

    // Representación visual en consola
    void print();
};

#endif // POLYNOMIAL_H