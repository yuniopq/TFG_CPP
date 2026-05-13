#include "Polynomial.h"

Polynomial::Polynomial(const GaloisField &galoisField, const std::vector<uint16_t> &coefficients)
    : coef(coefficients), gf(galoisField) { // Se copia el objeto ligero
    trim();
}

int Polynomial::getDegree() const
{
    if (coef.empty()) {
        return -1;
    }
    return static_cast<int>(coef.size()) - 1;
}

uint16_t Polynomial::getCoef(int degree) const
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

int Polynomial::setCoef(int degree, uint16_t value)
{
    if (degree < 0) {
        return -1;
    }

    size_t index = static_cast<size_t>(degree);
    if (index >= coef.size()) {
        coef.resize(index + 1, 0);
    }

    coef[index] = value;
    trim();
    return 0;
}

Polynomial Polynomial::add(const Polynomial &p2) const
{
    if (coef.empty()) {
        return Polynomial(gf, p2.coef);
    }
    if (p2.coef.empty()) {
        return Polynomial(gf, coef);
    }

    std::vector<uint16_t> result_coef(std::max(coef.size(), p2.coef.size()), 0);
    for (size_t i = 0; i < result_coef.size(); i++) {
        uint16_t c1 = (i < coef.size()) ? coef[i] : 0;
        uint16_t c2 = (i < p2.coef.size()) ? p2.coef[i] : 0;
        result_coef[i] = gf.add(c1, c2);
    }
    Polynomial res(gf, result_coef);
    // res.trim();
    return res;
}

Polynomial Polynomial::multiply(const Polynomial &p2) const{
    if (coef.empty() || p2.coef.empty()) {
        return Polynomial(gf, {});
    }

    std::vector<uint16_t> result_coef(coef.size() + p2.coef.size() - 1, 0);
    
    for(size_t i = 0; i < coef.size(); i++){
        for(size_t j=0; j<p2.coef.size(); j++){
            result_coef[i+j] = gf.add(result_coef[i+j], gf.multiply(coef[i], p2.coef[j]));
        }
    }
    Polynomial res(gf, result_coef);
    // res.trim();
    return res;
}

Polynomial Polynomial::operator+(const Polynomial &p2) const {
    return add(p2);
}

Polynomial Polynomial::operator*(const Polynomial &p2) const {
    return multiply(p2);
}

uint16_t Polynomial::evaluate(uint16_t x) {
    if (coef.empty()) {
        return 0;
    }
    if (x == 0) return getCoef(0);
    uint16_t res = 0;

    // Recorremos de mayor a menor grado: res = (...((a_n*x + a_n-1)*x + ...)*x + a_0)
    for (int i = getDegree(); i >= 0; i--) {
        res = gf.add(gf.multiply(res, x), getCoef(i));
    }
    return res;
}

void Polynomial::print() {
    if (coef.empty()) {
        std::cout << 0 << std::endl;
        return;
    }

    bool first_term = true;
    for (int i = getDegree(); i >= 0; i--) {
        int c = getCoef(i);
        if (c != 0) {
            if (!first_term) std::cout << " + "; // Print the + BEFORE the next term
            
            if (i == 0) {
                std::cout << c;
            } else {
                std::cout << c << "x^" << i;
            }
            first_term = false;
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
