#include <iostream>
#include <stdexcept>
#include "BCH_Codec.h"

BCH_Codec::BCH_Codec(int m, int t, int primitive_poly)
    : m(m), t(t), gf(m, primitive_poly), generator(gf, {}) {
    
    n = (1 << m) - 1;  // 2^m - 1

    computeGeneratorPolynomial();

    k = n - generator.getDegree();
    
    if (k <= 0) {
        throw std::invalid_argument("Invalid BCH parameters: k must be positive");
    }
    
}

void BCH_Codec::computeGeneratorPolynomial() {
    // Initialize generator as polynomial 1
    generator = Polynomial(gf, std::vector<uint16_t>{1});
    std::unordered_set<uint16_t> processed_exp;

    for (uint16_t i = 1; i <= 2 * t; i++) {
        if (processed_exp.count(i)) continue;

        Polynomial mi(gf, {1});
        uint16_t currExp = i;
        
        do {
            uint16_t root = gf.power(2, currExp); // a^i
            // mi(x) = mi(x) * (x + root)
            mi = mi * Polynomial(gf, {root, 1});
            
            processed_exp.insert(currExp);
            currExp = (currExp * 2) % n;
        } while (currExp != i);

        // Multiplicamos el generador acumulado por el nuevo polinomio mínimo
        generator = generator * mi;
    }
}

std::vector<uint16_t> BCH_Codec::encode(const std::vector<uint16_t>& message) {

    std::vector<uint16_t> parity(generator.getDegree(), 0);
    for (int i = message.size()-1; i>=0; i--){
        uint16_t MSB = parity.back();
        for(std::size_t j = parity.size()-1; j>0; j--){
            parity[j] = parity[j-1];
        }
        parity[0] = message[i];
        if(MSB == 1){
            for( std::size_t j=0; j<parity.size(); j++)
                parity[j]^=generator.getCoef(j);
        }
    }

    parity.insert(parity.end(), message.begin(), message.end());
    return parity;
}

std::vector<uint16_t> BCH_Codec::encodeLFSR(const std::vector<uint16_t> &message) {

    std::vector<uint16_t> parity(n - k, 0);
    for (int i = message.size()-1; i>=0; i--){
        uint16_t MSB = parity.back();
        uint16_t b = message[i];
        uint16_t feedback = MSB ^ b;
        if(feedback == 1){
            for (int j = parity.size()-1; j>0; j--){
                parity[j] = parity[j-1] ^ generator.getCoef(j);
            }
            parity[0] = generator.getCoef(0);
        }else{
            for (int j = parity.size()-1; j>0; j--){
                parity[j] = parity[j-1];
            }
            parity[0] = 0;
        }
    }
    parity.insert(parity.end(), message.begin(), message.end());
    return parity;
}

std::vector<uint16_t> BCH_Codec::syndrome(const std::vector<uint16_t> &received, bool &error) {
    Polynomial r(gf, received);
    std::vector<uint16_t> synd(2 * t + 1, 0);
    error = false;
    for(int i=1; i<=2*t; i++){
        if (i%2 == 0){
            synd[i] = gf.power(synd[i/2],2);
        } else{
            uint16_t root = gf.power(2, i); // a^i
            synd[i] = r.evaluate(root);
        }
        if(synd[i] != 0 and !error){
            error = true;
        }
    }
    return synd;
}

Polynomial BCH_Codec::belerkampMassey(const std::vector<uint16_t> synd)
{
    Polynomial B(gf, {1}), C(gf, {1});      
    int L=0, m=1, db=1;
    for(int i=1; i<=2*t; i++){

        int d=synd[i];

        for(int j=1; j<=L; j++){
            int term = gf.multiply(C.getCoef(j), synd[i-j]);
            d = gf.add(d, term);
        }

        if (d==0){
            m++;
        } else{
            //C(x) = C(x) + d/db * B(x) * x^m
            Polynomial tmpC = C;
            // Creamos un polinomio que representa d * db_inv * x^m * B(x)
            Polynomial correctionPoly(gf, std::vector<uint16_t>(m+1, 0));
            correctionPoly.setCoef( m , gf.multiply( d, gf.inverse(db)) );
            C = C + ( correctionPoly * B);

            if (2*L <= i-1){
                db=d;
                L = i - L;
                B = C;
                m=1;
            } else{
                m++;
            }
        }
    }
    return C;
}

std::vector<uint16_t> BCH_Codec::decode(const std::vector<uint16_t>& received) {
    // Placeholder: assume no errors and return message part
    // Full implementation would compute syndrome, find error locations, correct
    bool haveErrors;
    std::vector<uint16_t> synd = syndrome(received, haveErrors);

    if (!haveErrors) {
        std::cout << "No errors detected." << std::endl;
        std::vector<uint16_t> decoded(received.begin(), received.begin() + k);
        return decoded;
    } else {
        std::cout << "Errors detected, but correction not implemented." << std::endl;
    }
    size_t msg_size = std::min(static_cast<size_t>(k), received.size());
    std::vector<uint16_t> decoded(received.begin(), received.begin() + msg_size);
    
    return decoded;
}



void BCH_Codec::printGeneratorPolynomial() {
    if (generator.getDegree() >= 0) {
        std::cout << "Generator polynomial (degree " << generator.getDegree() << "): ";
        generator.print();
    } else {
        std::cout << "Generator polynomial not initialized" << std::endl;
    }
}