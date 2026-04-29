#include "Polynomial.h"

Polynomial::Polynomial(const GaloisField &galoisField, const vector<int> &coefficients): coef(coefficients), gf(&galoisField) {
    trim();
}

int Polynomial::getDegree() const
{
    if (coef.empty()) {
        return -1;
    }
    return static_cast<int>(coef.size()) - 1;
}

int Polynomial::getCoef(int degree) const
{
    if (degree < 0) {
        return 0;
    }

    size_t index = static_cast<size_t>(degree);
    if (index >= coef.size()) {
        return 0;
    }

    return coef[index];
}

int Polynomial::setCoef(int degree, int value)
{
    if (degree < 0) {
        return -1;
    }

    size_t index = static_cast<size_t>(degree);
    if (index >= coef.size()) {
        coef.resize(index + 1, 0);
    }

    coef[index] = value;
    return 0;
}

Polynomial Polynomial::add(const Polynomial &p2) const
{
    if (coef.empty()) {
        return Polynomial(*gf, p2.coef);
    }
    if (p2.coef.empty()) {
        return Polynomial(*gf, coef);
    }

    vector<int> result_coef(max(coef.size(), p2.coef.size()), 0);
    for (size_t i = 0; i < result_coef.size(); i++) {
        int c1 = (i < coef.size()) ? coef[i] : 0;
        int c2 = (i < p2.coef.size()) ? p2.coef[i] : 0;
        result_coef[i] = gf->add(c1, c2);
    }
    Polynomial res(*gf, result_coef);
    res.trim();
    return res;
}

Polynomial Polynomial::multiply(const Polynomial &p2) const{
    if (coef.empty() || p2.coef.empty()) {
        return Polynomial(*gf, {});
    }

    vector<int> result_coef(coef.size() + p2.coef.size() - 1, 0);
    
    for(size_t i = 0; i < coef.size(); i++){
        for(size_t j=0; j<p2.coef.size(); j++){
            result_coef[i+j] = gf->add(result_coef[i+j], gf->multiply(coef[i], p2.coef[j]));
        }
    }
    Polynomial res(*gf, result_coef);
    res.trim();
    return res;
}

Polynomial Polynomial::operator+(const Polynomial &p2) const {
    return add(p2);
}

Polynomial Polynomial::operator*(const Polynomial &p2) const {
    return multiply(p2);
}

int Polynomial::evaluate(int x) {
    if (coef.empty()) {
        return 0;
    }
    if (x == 0) return getCoef(0);
    int res = 0;
    
    // Recorremos de mayor a menor grado: res = (...((a_n*x + a_n-1)*x + ...)*x + a_0)
    for (int i = getDegree(); i >= 0; i--) {
        res = gf->add(gf->multiply(res, x), getCoef(i));
    }
    return res;
}

void Polynomial::print(){
    if (coef.empty()) {
        std::cout << 0 << std::endl;
        return;
    }

    for (int i = getDegree(); i >= 0; i--) {
        int coef = getCoef(i);
        if (coef != 0) {
            if (i == 0) {
                std::cout << coef;
            } else {
                std::cout << coef << "x^" << i;
            }
            if (i > 0) {
                std::cout << " + ";
            }
        }
    }
    std::cout << std::endl;
}

// Remove trailing zero coefficients so degree reflects highest non-zero term
void Polynomial::trim() {
    while (!coef.empty() && coef.back() == 0) {
        coef.pop_back();
    }
}
