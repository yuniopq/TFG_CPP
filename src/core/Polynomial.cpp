#include "Polynomial.h"

Polynomial::Polynomial(const GaloisField &galoisField, const vector<int> &coefficients): gf(const_cast<GaloisField&>(galoisField)), coef(coefficients) {
}

int Polynomial::getDegree() const
{
    return coef.size() - 1;
}

int Polynomial::getCoef(int degree) const
{
    if (degree < 0 || degree >= coef.size()) {
        return 0; // Coefficient is 0 for degrees outside the current size
    }
    return coef[degree];
}

int Polynomial::setCoef(int degree, int value)
{
    if (degree < 0) {
        return -1; // Invalid degree
    }
    if (degree >= coef.size()) {
        coef.resize(degree + 1, 0); // Resize and fill new coefficients with 0
    }
    coef[degree] = value;
    return 0; 
}

Polynomial Polynomial::add(const Polynomial &p2)
{
    vector<int> result_coef(max(coef.size(), p2.coef.size()), 0);
    for (size_t i = 0; i < result_coef.size(); i++) {
        int c1 = (i < coef.size()) ? coef[i] : 0;
        int c2 = (i < p2.coef.size()) ? p2.coef[i] : 0;
        result_coef[i] = c1^c2;
    }
    return Polynomial(gf, result_coef);
}

Polynomial Polynomial::multiply(const Polynomial &p2){
    vector<int> result_coef(coef.size() + p2.coef.size() - 1, 0);
    
    for(size_t i = 0; i < coef.size(); i++){
        for(size_t j=0; j<p2.coef.size(); j++){
            result_coef[i+j] = gf.add(result_coef[i+j], gf.multiply(coef[i], p2.coef[j]));
        }
    }
    return Polynomial(gf, result_coef);
}

int Polynomial::evaluate(int x) {
    if (x == 0) return getCoef(0);
    int res = 0;
    
    // Recorremos de mayor a menor grado: res = (...((a_n*x + a_n-1)*x + ...)*x + a_0)
    for (int i = getDegree(); i >= 0; i--) {
        res = gf.add(gf.multiply(res, x), getCoef(i));
    }
    return res;
}