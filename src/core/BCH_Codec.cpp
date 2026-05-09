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

        // Multiplicamos el generador acumulado por el nuevo polinomio mínimo
        generator = generator * mi;
    }
}

// ============================================================================
// VERSIÓN ANTIGUA (vector<uint16_t>, sin bitpacking) 
// ============================================================================
/*
std::vector<uint16_t> BCH_Codec::encode_OLD(const std::vector<uint16_t>& message) {
    if (message.size() != static_cast<size_t>(k)) {
        throw std::invalid_argument("Message length must be exactly k");
    }

    int g = generator.getDegree();

    // 1. Crear mensaje extendido: m(x)*x^r
    std::vector<uint16_t> buffer = message;
    buffer.insert(buffer.begin(), g, 0); // añadir ceros al inicio

    // 2. División polinómica
    for (int i = buffer.size() - 1; i >= g; i--) {
        if (buffer[i] == 1) { // si hay que dividir
            for (int j = 0; j <= g; j++) {
                buffer[i - j] ^= generator.getCoef(g - j);
            }
        }
    }

    // 3. El resto son los primeros g bits
    std::vector<uint16_t> parity(buffer.begin(), buffer.begin() + g);

    // 4. Código final: parity + mensaje
    parity.insert(parity.end(), message.begin(), message.end());

    return parity;
}
*/
// ============================================================================
// VERSIÓN NUEVA (PackedBits, optimizada)
// ============================================================================
std::vector<uint16_t> BCH_Codec::encode(const std::vector<uint16_t>& message) {
    if (message.size() != static_cast<size_t>(k)) {
        throw std::invalid_argument("Message length must be exactly k");
    }

    int g = generator.getDegree();

    // 1. Crear mensaje extendido: m(x) * x^g con bits empaquetados
    std::vector<uint16_t> buffer(g, 0);
    buffer.insert(buffer.end(), message.begin(), message.end());
    PackedBit binaryBuffer(buffer);

    // Polinomio generador binario (coeficientes por grado, de 0 a g)
    // O eso es lo que se pensaba, pero debido a una condicion de parada en el algoritmo de desplazamiento
    // El polinomio generador será igual de grande que el mensaje a enviar(con los bits de paridad)
    std::vector<uint16_t> gen_bits(g+1, 0);
    for (int deg = 0; deg <= g; ++deg) {
        gen_bits[deg] = generator.getCoef(deg);
    }
    PackedBit gen_packed(gen_bits);

    // 2. División polinómica: si msb es 1, XOR de g(x) alineado al grado actual
    for (int i = buffer.size() - 1; i >= g; --i) {
        if (binaryBuffer.get(i)) {
            size_t shift = (i - g);
            // binaryBuffer ^= gen_packed << shift;
            binaryBuffer.xorShifted(gen_packed, shift);
        }
    }

    // 3. El resto son los primeros g bits
    buffer = binaryBuffer.toVector();
    std::vector<uint16_t> parity(buffer.begin(), buffer.begin() + g);

    // 4. Código final: parity + mensaje
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

    // Procesamos el mensaje de mayor a menor grado (Horner)
    for (int i = message.size() - 1; i >= 0; i--) {
        // El bit que "sale" por la izquierda al multiplicar por x
        uint16_t feedback = message[i] ^ remainder[g_deg - 1];

        // Desplazamiento y XOR (La esencia de Horner en campos finitos)
        for (int j = g_deg - 1; j > 0; j--) {
            remainder[j] = remainder[j - 1] ^ (feedback ? generator.getCoef(j) : 0);
        }
        remainder[0] = (feedback ? generator.getCoef(0) : 0);
    }
    
    // Al final, 'remainder' es el resto de la división
    // Combinar con mensaje para forma sistemática...
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
        if(synd[i] != 0 and !error){
            error = true;
        }
    }
    return synd;
}

Polynomial BCH_Codec::berlekampMassey(const std::vector<uint16_t> synd)
{
    Polynomial B(gf, {1}), C(gf, {1});      
    int L=0, shift_m=1, db=1;
    for(int i=1; i<=2*t; i++){

        int d=synd[i];

        for(int j=1; j<=L; j++){
            int term = gf.multiply(C.getCoef(j), synd[i-j]);
            d = gf.add(d, term);
        }

        if (d==0){
            shift_m++;
        } else{
            //C(x) = C(x) + d/db * B(x) * x^m
            Polynomial tmpC = C;
            // Creamos un polinomio que representa d * db_inv * x^m * B(x)
            Polynomial correctionPoly(gf, std::vector<uint16_t>(m+1, 0));
            correctionPoly.setCoef( shift_m , gf.multiply( d, gf.inverse(db)) );
            C = C + ( correctionPoly * B);

            if (2*L <= i-1){
                db=d;
                L = i - L;
                B = tmpC;
                shift_m=1;
            } else{
                shift_m++;
            }
        }
    }
    return C;
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
            std::cerr << "[!] Errores incorregibles: L=" << L << " y t=" << t << std::endl;
        }else{

            // Chien's search
            
            std::vector<uint16_t> reg(L+1, 0);

            for (int i=0; i<=L; i++){
                reg[i] = errorLocator.getCoef(i);
            }

            std::vector<uint16_t> factors(L + 1);
            for (int i = 0; i <= L; i++){
                factors[i] = gf.power(2, n-i); // a^(-j)
            }
            

            int rootsFound = 0;
            for (int i = 0; i < n; i++) {
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
                std::cerr << "[!] Advertencia: Se esperaban " << L << " errores, pero se encontraron " << rootsFound << std::endl;
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

    // 1. Calcular los síndromes para ver si hay errores en el canal
    std::vector<uint16_t> synd = syndrome(corrected, errorsDetected);

    // Si no hay errores, extraemos el mensaje y reportamos éxito rotundo
    if (!errorsDetected) {
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return true; // Éxito: Bloque limpio
    }

    // 2. Si hay errores, aplicamos el algoritmo de Berlekamp-Massey
    Polynomial errorLocator = berlekampMassey(synd);
    int L = errorLocator.getDegree();

    // 3. Comprobación matemática de errores incorregibles
    if (L == 0 || L > t) {
        // Fallo: Demasiados errores. Extraemos el mensaje tal cual (con fallos)
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return false; // FER = 1
    }

    // 4. Búsqueda de Chien para encontrar la posición de los errores
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
            corrected[i] ^= 1; // Voltea el bit erróneo
            rootsFound++;
        }

        // Actualizar registros para la siguiente iteración
        for (int j = 0; j <= L; j++) {
            reg[j] = gf.multiply(reg[j], factors[j]);
        }
    }

    // 5. Comprobación de integridad de la búsqueda de Chien
    if (rootsFound != L) {
        // Fallo: El polinomio decía que había L errores, pero encontramos menos/más.
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return false; 
    }

    // 6. Decodificación exitosa (el BCH logró arreglar el bloque)
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