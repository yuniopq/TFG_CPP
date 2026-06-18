#ifndef PACKEDBIT_H
#define PACKEDBIT_H
#include <vector>
#include <cstdint>
#include <algorithm>

class PackedBit {
public:
    std::vector<uint64_t> data;
    size_t bitCount;

    PackedBit(const std::vector<uint16_t> &original) {
        bitCount = original.size();
        size_t blockCount = (bitCount + 63) / 64;
        data.resize(blockCount, 0);
        for (size_t i = 0; i < bitCount; ++i) {
            if (original[i]) {
                data[i / 64] |= (1ULL << (i % 64));
            }
        }
    }

    uint16_t get(size_t index) const {
        assert(index < bitCount && "Índice fuera de rango en PackedBit");
        return (data[index / 64] >> (index % 64)) & 1ULL;
    }

    std::vector<uint16_t> toVector() {
        std::vector<uint16_t> res(bitCount, 0);
        size_t fullBlocks = bitCount / 64;
        uint64_t blockData;
        
        for (size_t block = 0; block < fullBlocks; block++) {
            blockData = data[block];
            for (size_t offset = 0; offset < 64; offset++) {
                res[block * 64 + offset] = (blockData >> offset) & 1ULL;
            }
        }
        
        // Procesamiento de los bits sobrantes del último bloque
        size_t remainingBits = bitCount % 64;
        if (remainingBits > 0) {
            blockData = data[fullBlocks];
            for (size_t offset = 0; offset < remainingBits; offset++) {
                res[fullBlocks * 64 + offset] = (blockData >> offset) & 1ULL;
            }
        }   
        return res;
    }

    void xorShifted(const PackedBit &gx, size_t shift_bits) {
        assert(shift_bits < bitCount && !gx.data.empty() && "Parámetros inválidos en xorShifted");

        size_t block_shift = shift_bits / 64;
        size_t bit_shift = shift_bits % 64;
        uint64_t carry = 0, next_carry = 0;
        
        size_t dst;
        for (size_t i = 0; i < gx.data.size(); ++i) {
            dst = i + block_shift;
            if (dst >= data.size()) break;

            uint64_t word = gx.data[i];

            // Prevención de comportamiento indefinido: shift >> 64
            next_carry = (bit_shift == 0) ? 0 : (word >> (64 - bit_shift));

            // Alineación de grado mediante desplazamiento intrabloque y acarreo
            data[dst] ^= (word << bit_shift) | carry;
            carry = next_carry;
        }

        // Adición del último acarreo al bloque adyacente
        dst = gx.data.size() + block_shift;
        if (carry && dst < data.size()) {
            data[dst] ^= carry;
        }

        // Limpieza de máscara final
        if (bitCount % 64 != 0 && !data.empty()) {
            uint64_t keep_mask = (1ULL << (bitCount % 64)) - 1ULL;
            data.back() &= keep_mask;
        }   
    }
};

#endif // PACKEDBIT_H