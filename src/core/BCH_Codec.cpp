#include "../../include/BCH_Codec.h"
#include <iostream>
#include <stdexcept>

BCH_Codec::BCH_Codec(int m, int t, int primitive_poly)
    : m(m), t(t), gf(m, primitive_poly), generator(nullptr) {
    
    n = (1 << m) - 1;  // 2^m - 1

    computeGeneratorPolynomial();

    k = n - generator->getDegree();
    
    if (k <= 0) {
        throw invalid_argument("Invalid BCH parameters: k must be positive");
    }
    
}

void BCH_Codec::computeGeneratorPolynomial() {
    // Initialize generator as polynomial 1
    generator = make_unique<Polynomial>(gf, vector<int>{1});
    std::unordered_set<int> processed_exp;

    for (int i = 1; i <= 2 * t; i++) {
        if (processed_exp.count(i)) continue;

        Polynomial mi(gf, {1});
        int currExp = i;
        
        do {
            int root = gf.power(2, currExp); // a^i
            // mi(x) = mi(x) * (x + root)
            mi = mi * Polynomial(gf, {root, 1});
            
            processed_exp.insert(currExp);
            currExp = (currExp * 2) % n;
        } while (currExp != i);

        // Multiplicamos el generador acumulado por el nuevo polinomio mínimo
        *generator = (*generator) * mi;
    }
}

vector<int> BCH_Codec::encode(const vector<int>& message) {

    vector<int> parity(generator->getDegree(), 0);
    for (int i = message.size()-1; i>=0; i--){
        int MSB = parity.back();
        for(int j = parity.size()-1; j>0; j--){
            parity[j] = parity[j-1];
        }
        parity[0] = message[i];
        if(MSB == 1){
            for( int j=0; j<parity.size(); j++)
                parity[j]^=generator->getCoef(i);
        }
    }

    parity.insert(parity.end(), message.begin(), message.end());
    return parity;
}

vector<int> BCH_Codec::decode(const vector<int>& received) {
    // Placeholder: assume no errors and return message part
    // Full implementation would compute syndrome, find error locations, correct
    
    size_t msg_size = min(static_cast<size_t>(k), received.size());
    vector<int> decoded(received.begin(), received.begin() + msg_size);
    
    return decoded;
}

void BCH_Codec::printGeneratorPolynomial() {
    if (generator) {
        std::cout << "Generator polynomial degree: " << generator->getDegree() << std::endl;
    } else {
        std::cout << "Generator polynomial not initialized" << std::endl;
    }
}
