#include <iostream>
#include <stdexcept>
#include <cassert>
#include "BCH_Codec.h"
#include "PackedBit.h"

BCH_Codec::BCH_Codec(int m, int t, int primitive_poly)
    : m(m), t(t), gf(m, primitive_poly), generator(gf, {}) {
    
    if (m < 3 || m > 15) {
        throw std::invalid_argument("Parámetros BCH inválidos: m debe estar entre 1 y 15");
    }

    n = (1 << m) - 1;  // n = 2^m - 1

    if (t <= 0 || 2LL * t >= n) {
        throw std::invalid_argument("Parámetros BCH inválidos: t debe cumplir 1 <= 2t < n");
    }

    computeGeneratorPolynomial();

    k = n - generator.getDegree();
    
    if (k <= 0) {
        throw std::invalid_argument("Parámetros BCH inválidos: k debe ser positivo");
    }
}

void BCH_Codec::computeGeneratorPolynomial() {
    // Inicializa el generador como polinomio 1
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

        // Multiplica el generador acumulado por el nuevo polinomio mínimo
        generator = generator * mi;
    }
}

std::vector<uint16_t> BCH_Codec::encode(const std::vector<uint16_t>& message) {
    assert(message.size() == static_cast<size_t>(k) && "Longitud del mensaje incorrecta");

    int g = generator.getDegree();

    // 1. Mensaje extendido: m(x) * x^g empaquetado en bits
    std::vector<uint16_t> buffer(g, 0);
    buffer.insert(buffer.end(), message.begin(), message.end());
    PackedBit binaryBuffer(buffer);

    // Polinomio generador binario
    std::vector<uint16_t> gen_bits(g+1, 0);
    for (int deg = 0; deg <= g; ++deg) {
        gen_bits[deg] = generator.getCoef(deg);
    }
    PackedBit gen_packed(gen_bits);

    // 2. División polinómica: si el MSB es 1, se aplica XOR con g(x) alineado
    for (int i = buffer.size() - 1; i >= g; --i) {
        if (binaryBuffer.get(i)) {
            size_t shift = (i - g);
            binaryBuffer.xorShifted(gen_packed, shift);
        }
    }

    // 3. El resto de la división conforma los bits de paridad
    buffer = binaryBuffer.toVector();
    std::vector<uint16_t> parity(buffer.begin(), buffer.begin() + g);

    // 4. Palabra código final (sistemática): paridad + mensaje
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
        if (synd[i] != 0 and !error){
            error = true;
        }
    }
    return synd;
}

Polynomial BCH_Codec::berlekampMassey(const std::vector<uint16_t>& synd) {
    // Uso directo de vectores para evitar la sobrecarga de instanciar objetos Polynomial
    std::vector<uint16_t> C(2 * t + 2, 0);
    std::vector<uint16_t> B(2 * t + 2, 0);
    C[0] = 1;
    B[0] = 1;
    int L = 0;
    int shift_m = 1;
    uint16_t db = 1; // Discrepancia previa

    for (int i = 1; i <= 2 * t; i++) {
        // 1. Calcula la discrepancia d
        uint16_t d = synd[i];
        int max_j = std::min(L, (int)C.size() - 1);
        for (int j = 1; j <= max_j; j++) {
            d = gf.add(d, gf.multiply(C[j], synd[i - j]));
        }

        if (d == 0) {
            shift_m++;
        } else {
            std::vector<uint16_t> oldC = C;
            
            // 2. Actualiza C(x) = C(x) + (d/db) * x^shift_m * B(x)
            uint16_t factor = gf.multiply(d, gf.inverse(db));
            
            for (size_t j = 0; j + shift_m < B.size(); j++) {
                C[j + shift_m] = gf.add(C[j + shift_m], gf.multiply(factor, B[j]));
            }

            // 3. Actualiza la estimación L
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
    // El constructor de Polynomial llamará a trim() automáticamente
    return Polynomial(gf, C);
}

bool BCH_Codec::decode(const std::vector<uint16_t>& received, std::vector<uint16_t>& message_out) {
    assert(received.size() == static_cast<size_t>(n) && "Longitud de palabra recibida incorrecta");

    std::vector<uint16_t> corrected = received;
    bool errorsDetected = false;

    // 1. Calcula los síndromes para detectar errores en el canal
    std::vector<uint16_t> synd = syndrome(corrected, errorsDetected);

    // Si no hay errores, se extrae el mensaje directamente
    if (!errorsDetected) {
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return true; // Éxito: bloque limpio
    }

    // 2. Si hay errores, se aplica Berlekamp-Massey
    Polynomial errorLocator = berlekampMassey(synd);
    int L = errorLocator.getDegree();

    // 3. Verificación de errores incorregibles
    if (L == 0 || L > t) {
        // Fallo: demasiados errores. Se devuelve el mensaje sin corregir
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return false;
    }

    // 4. Búsqueda de Chien para encontrar las posiciones de los errores
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
            corrected[i] ^= 1; // Invierte el bit erróneo
            rootsFound++;
        }

        // Actualiza los registros para la siguiente iteración
        for (int j = 0; j <= L; j++) {
            reg[j] = gf.multiply(reg[j], factors[j]);
        }
    }

    // 5. Comprobación de integridad de la búsqueda de Chien
    if (rootsFound != L) {
        // Fallo: discrepancia entre los errores estimados y los encontrados
        message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
        return false; 
    }

    // 6. Decodificación exitosa
    message_out = std::vector<uint16_t>(corrected.end() - k, corrected.end());
    return true; 
}

void BCH_Codec::printGeneratorPolynomial() {
    if (generator.getDegree() >= 0) {
        std::cout << "Polinomio generador (grado " << generator.getDegree() << "): ";
        generator.print();
    } else {
        std::cout << "Polinomio generador no inicializado" << std::endl;
    }
}