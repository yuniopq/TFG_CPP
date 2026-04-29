#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <vector>
#include "GaloisField.h"

using namespace std;

class Polynomial{
private:    
    vector<int> coef;       // coef[0] + coef[1]·x + coef[2]·x^2 + ...
    GaloisField& gf;

public:
    Polynomial(const GaloisField& galoisField, const vector<int>& coefficients);

    int getDegree() const;
    int getCoef(int degree) const;
    int setCoef(int degree, int value);

    Polynomial add(const Polynomial& p2);
    Polynomial multiply(const Polynomial& p2);

    int evaluate(int x);

    void print();

};
#endif