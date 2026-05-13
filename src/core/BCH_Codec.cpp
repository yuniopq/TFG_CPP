#include <iostream>
#include <stdexcept>
#include "BCH_Codec.h"
#include "PackedBit.h"

BCH_Codec::BCH_Codec(int m, int t, int primitive_poly)
    : m(m), t(t), gf(m, primitive_poly), generator(gf, {}) {
    
    if (m < 1 || m > 15) {
        throw std::invalid_argument("Invalid BCH parameters: m must be between 1 and 15");
    }

    n = (1 << m) - 1;  // 2^m - 1

    if (t <= 0 || 2LL * t >= n) {
        throw std::invalid_argument("Invalid BCH parameters: t must satisfy 1 <= 2t < n");
    }

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

        // Multiply the accumulated generator by the new minimal polynomial
        generator = generator * mi;
    }
}

// ============================================================================
// OLD VERSION (vector<uint16_t>, no bit packing)
// ============================================================================
/*
std::vector<uint16_t> BCH_Codec::encode_OLD(const std::vector<uint16_t>& message) {
    if (message.size() != static_cast<size_t>(k)) {
        throw std::invalid_argument("Message length must be exactly k");
    }

    int g = generator.getDegree();

    // 1. Create extended message: m(x) * x^r
    std::vector<uint16_t> buffer = message;
    buffer.insert(buffer.begin(), g, 0); // prepend zeros

    // 2. Polynomial division
    for (int i = buffer.size() - 1; i >= g; i--) {
            if (buffer[i] == 1) { // if division is needed
            for (int j = 0; j <= g; j++) {
                buffer[i - j] ^= generator.getCoef(g - j);
            }
        }
    }

    // 3. The remainder is the first g bits
    std::vector<uint16_t> parity(buffer.begin(), buffer.begin() + g);

    // 4. Final codeword: parity + message
    parity.insert(parity.end(), message.begin(), message.end());

    return parity;
}
*/
// ============================================================================
// NEW VERSION (PackedBits, optimized)
// ============================================================================
std::vector<uint16_t> BCH_Codec::encode(const std::vector<uint16_t>& message) {
    if (message.size() != static_cast<size_t>(k)) {
        throw std::invalid_argument("Message length must be exactly k");
    }

    int g = generator.getDegree();

    // 1. Create extended message: m(x) * x^g with packed bits
    std::vector<uint16_t> buffer(g, 0);
    buffer.insert(buffer.end(), message.begin(), message.end());
    PackedBit binaryBuffer(buffer);

    // Binary generator polynomial (coefficients by degree, from 0 to g)
    // Or so it was thought, but due to a stopping condition in the shift algorithm
    // The generator polynomial will be as large as the message to send (with parity bits)
    std::vector<uint16_t> gen_bits(g+1, 0);
    for (int deg = 0; deg <= g; ++deg) {
        gen_bits[deg] = generator.getCoef(deg);
    }
    PackedBit gen_packed(gen_bits);

    // 2. Polynomial division: if MSB is 1, XOR g(x) aligned to the current degree
    for (int i = buffer.size() - 1; i >= g; --i) {
        if (binaryBuffer.get(i)) {
            size_t shift = (i - g);
            // binaryBuffer ^= gen_packed << shift;
            binaryBuffer.xorShifted(gen_packed, shift);
        }
    }

    // 3. The remainder is the first g bits
    buffer = binaryBuffer.toVector();
    std::vector<uint16_t> parity(buffer.begin(), buffer.begin() + g);

    // 4. Final codeword: parity + message
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
std::vector<uint16_t> BCH_Codec::encodeHorner(const std::vector<uint16_t> &message) {
    int g_deg = generator.getDegree();
    std::vector<uint16_t> remainder(g_deg, 0);

    // Process the message from highest to lowest degree (Horner)
    for (int i = message.size() - 1; i >= 0; i--) {
        // El bit que "sale" por la izquierda al multiplicar por x
        uint16_t feedback = message[i] ^ remainder[g_deg - 1];

        // Desplazamiento y XOR (La esencia de Horner en campos finitos)
        for (int j = g_deg - 1; j > 0; j--) {
            remainder[j] = remainder[j - 1] ^ (feedback ? generator.getCoef(j) : 0);
        }
        remainder[0] = (feedback ? generator.getCoef(0) : 0);
    }
    
    // At the end, 'remainder' is the division remainder
    // Combine with the message to form the systematic codeword...
    remainder.insert(remainder.end(), message.begin(), message.end());

    return remainder;
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
        if (synd[i] != 0 and !error){
            error = true;
        }
    }
    return synd;
}

// Polynomial BCH_Codec::berlekampMassey(const std::vector<uint16_t> synd)
// {
//     Polynomial B(gf, {1}), C(gf, {1});      
//     int L=0, shift_m=1, db=1;
//     for(int i=1; i<=2*t; i++){

//         int d=synd[i];

//         for(int j=1; j<=L; j++){
//             int term = gf.multiply(C.getCoef(j), synd[i-j]);
//             d = gf.add(d, term);
//         }

//         if (d==0){
//             shift_m++;
//         } else{
//             //C(x) = C(x) + d/db * B(x) * x^m
//             Polynomial tmpC = C;
//             // Create a polynomial representing d * db_inv * x^m * B(x)
//             Polynomial correctionPoly(gf, std::vector<uint16_t>(m+1, 0));
//             correctionPoly.setCoef( shift_m , gf.multiply( d, gf.inverse(db)) );
//             C = C + ( correctionPoly * B);

//             if (2*L <= i-1){
//                 db=d;
//                 L = i - L;
//                 B = tmpC;
//                 shift_m=1;
//             } else{
//                 shift_m++;
//             }
//         }
//     }
//     return C;
// }
Polynomial BCH_Codec::berlekampMassey(const std::vector<uint16_t> synd) {
    // Use vectors directly to avoid Polynomial overhead and trim()
    std::vector<uint16_t> C = {1}; 
    std::vector<uint16_t> B = {1}; 
    int L = 0;
    int shift_m = 1;
    uint16_t db = 1; // Discrepancia previa

    for (int i = 1; i <= 2 * t; i++) {
        // 1. Compute discrepancy d
        // uint16_t d = synd[i];
        // for (int j = 1; j <= L; j++) {
        //     if (j < (int)C.size()) {
        //         d = gf.add(d, gf.multiply(C[j], synd[i - j]));
        //     }
        // }
        // Inside the loop for i = 1 to 2*t
        uint16_t d = synd[i];
        int max_j = std::min(L, (int)C.size() - 1);
        for (int j = 1; j <= max_j; j++) {
            d = gf.add(d, gf.multiply(C[j], synd[i - j]));
        }

        if (d == 0) {
            shift_m++;
        } else {
            std::vector<uint16_t> oldC = C; // Copy for B update
            
            // 2. Update C(x) = C(x) + (d/db) * x^shift_m * B(x)
            uint16_t factor = gf.multiply(d, gf.inverse(db));
            
            size_t neededSize = B.size() + shift_m;
            if (C.size() < neededSize) {
                C.resize(neededSize, 0);
            }

            for (size_t j = 0; j < B.size(); j++) {
                C[j + shift_m] = gf.add(C[j + shift_m], gf.multiply(factor, B[j]));
            }

            // 3. Verify L update
            if (2 * L <= i - 1) {
                L = i - L;
                B = oldC;
                db = d;
                shift_m = 1;
            } else {
                shift_m++;
            }
        }
    }
    // At the end, the Polynomial constructor will call trim() once
    return Polynomial(gf, C);
}
std::vector<uint16_t> BCH_Codec::decode(const std::vector<uint16_t>& received) {
    if (received.size() != static_cast<size_t>(n)) {
        throw std::invalid_argument("Received word length must be exactly n");
    }

    std::vector<uint16_t> corrected = received;

    bool haveErrors;
    std::vector<uint16_t> synd = syndrome(corrected, haveErrors);

    if (haveErrors){
        Polynomial errorLocator = berlekampMassey(synd);
        int L = errorLocator.getDegree();
        if (L==0 or L>t){
            std::cerr << "[!] Uncorrectable errors: L=" << L << " and t=" << t << std::endl;
        }else{

            // Chien search
            
            std::vector<uint16_t> reg(L+1, 0);

            for (int i=0; i<=L; i++){
                reg[i] = errorLocator.getCoef(i);
            }

            std::vector<uint16_t> factors(L + 1);
            for (int i = 0; i <= L; i++){
                factors[i] = gf.power(2, n-i); // a^(-j)
            }
            

            int rootsFound = 0;
            for (int i = 0; i < n and rootsFound < L; i++) {
                uint16_t sum = 0;
                for (int j = 0; j <= L; j++){
                    sum = gf.add(sum, reg[j]);
                }

                if (sum==0){
                    corrected[i] ^= 1; // Flip the bit at position i
                    rootsFound++;
                }

                for (int j = 0; j <= L; j++){
                    reg[j] = gf.multiply(reg[j], factors[j]);
                }
                
            }
            if (rootsFound != L){
                std::cerr << "[!] Warning: Expected " << L << " errors, but found " << rootsFound << std::endl;
            }
        }
    }
         
    return std::vector<uint16_t>(corrected.end() - k, corrected.end());
}

bool BCH_Codec::decode(const std::vector<uint16_t>& received, std::vector<uint16_t>& message_out) {
    if (received.size() != static_cast<size_t>(n)) {
        throw std::invalid_argument("Received word length must be exactly n");
    }

    std::vector<uint16_t> corrected = received;
    bool errorsDetected = false;

    // 1. Compute syndromes to detect channel errors
    std::vector<uint16_t> synd = syndrome(corrected, errorsDetected);

    // If there are no errors, extract the message and report success
    if (!errorsDetected) {
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return true; // Success: clean block
    }

    // 2. If there are errors, apply Berlekamp-Massey
    Polynomial errorLocator = berlekampMassey(synd);
    int L = errorLocator.getDegree();

    // 3. Mathematical check for uncorrectable errors
    if (L == 0 || L > t) {
        // Failure: too many errors. Return the message as-is (with errors)
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return false; // CWER = 1
    }

    // 4. Chien search to find the error positions
    std::vector<uint16_t> reg(L + 1, 0);
    for (int i = 0; i <= L; i++) {
        reg[i] = errorLocator.getCoef(i);
    }

    std::vector<uint16_t> factors(L + 1);
    for (int i = 0; i <= L; i++) {
        factors[i] = gf.power(2, n - i); // a^(-j)
    }

    int rootsFound = 0;
    for (int i = 0; i < n; i++) {
        uint16_t sum = 0;
        for (int j = 0; j <= L; j++) {
            sum = gf.add(sum, reg[j]);
        }

        if (sum == 0) {
            corrected[i] ^= 1; // Flip the erroneous bit
            rootsFound++;
        }

        // Update registers for the next iteration
        for (int j = 0; j <= L; j++) {
            reg[j] = gf.multiply(reg[j], factors[j]);
        }
    }

    // 5. Integrity check for the Chien search
    if (rootsFound != L) {
        // Failure: the polynomial said there were L errors, but we found fewer/more.
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return false; 
    }

    // 6. Successful decoding (the BCH corrected the block)
    message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
    return true; 
}

void BCH_Codec::printGeneratorPolynomial() {
    if (generator.getDegree() >= 0) {
        std::cout << "Generator polynomial (degree " << generator.getDegree() << "): ";
        generator.print();
    } else {
        std::cout << "Generator polynomial not initialized" << std::endl;
    }
}