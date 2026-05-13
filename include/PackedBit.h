#ifndef PACKEDBIT_H
#define PACKEDBIT_H
#include <vector>
#include <cstdint>
#include <algorithm>

class PackedBit{
public:
    std::vector<uint64_t> data;
    size_t bitCount;

    PackedBit(const std::vector<uint16_t> &original){
        bitCount = original.size();
        size_t blockCount = (bitCount + 63) / 64;
        data.resize(blockCount, 0);
        for (size_t i=0; i<bitCount; ++i){
            if (original[i]){
                data[ i/64 ] |= (1ULL << (i % 64));
            }
        }
    }

    uint16_t get(size_t index) const {
        if (index >= bitCount) return 0;
        return (data[ index/64 ] >> (index % 64)) & 1ULL;
    }

    void set(size_t index, uint16_t value){
        if (index >= bitCount) return;
        if (value)
            data[ index/64 ] |= 1ULL << (index%64);
        else
            data[ index/64 ] &= ~(1ULL << (index%64));
    }

    std::vector<uint16_t> toVectorOLD(){
        std::vector<uint16_t> res(bitCount, 0);
        for(size_t i=0; i<bitCount; ++i){
            res[i] = (data[i/64] >> (i%64)) & 1ULL;
        }
        return res;
    }

    std::vector<uint16_t> toVector(){
        std::vector<uint16_t> res(bitCount, 0);
        size_t fullBlocks = bitCount / 64;
        uint64_t blockData;
        for(size_t block=0; block<fullBlocks; block++){
            blockData = data[block];
            for(size_t offset=0; offset<64; offset++){
                res[block*64 + offset] = (blockData >> offset) & 1ULL;
            }
        }
        // Handle remaining bits
        size_t remainingBits = bitCount % 64;
        if (remainingBits > 0) {
            uint64_t blockData = data[fullBlocks];
            for(size_t offset=0; offset<remainingBits; offset++){
                res[fullBlocks*64 + offset] = (blockData >> offset) & 1ULL;
            }
        }   
        return res;
    }

    void XOR(const PackedBit &other){
        size_t minBlocks = std::min(data.size(), other.data.size());
        for(size_t i=0; i<minBlocks; ++i){
            data[i] ^= other.data[i];
        }
    }

    PackedBit& operator^=(const PackedBit& other) {
        this->XOR(other); 
        return *this;
    }

    void shiftLeft(size_t shift){
        // Base cases: no movement needed, or we move more bits than exist
        if (shift == 0 || bitCount == 0) return;
        if (shift >= bitCount) {
            std::fill(data.begin(), data.end(), 0); // Todo se vuelve ceros
            return;
        }
        size_t blockShift = shift / 64;
        size_t bitShift = shift % 64;
        
        // Move complete blocks
        if (blockShift > 0){
            for (size_t i = data.size(); i > 0; --i){
                size_t idx = i - 1;
                if ( idx>= blockShift)
                    data[idx] = data[idx - blockShift];
                else 
                    data[idx] = 0;
            }
        }
        
        // Move bits within the blocks
        if (bitShift > 0){
            uint64_t carry = 0, next_carry = 0;
            for (size_t i=0; i<data.size(); ++i){
                next_carry = data[i]>>(64-bitShift);
                data[i] = (data[i]<<bitShift) | carry;
                carry = next_carry;
            }
        }

        // Limpieza
        if (bitCount % 64){
            uint64_t mask = (1ULL << (bitCount % 64)) - 1;
            data.back() &= mask;
        }
    }
    
    PackedBit& operator<<=(size_t shift) {
        this->shiftLeft(shift); 
        return *this;
    }

    PackedBit operator<<(size_t shift) const {
        PackedBit result = *this; // Hacemos la copia limpia
        result <<= shift;         // Desplazamos la copia
        return result;            // Devolvemos la copia desplazada
    }

    void xorShifted(const PackedBit &gx, size_t shift_bits) {
        if (shift_bits >= bitCount || gx.data.empty()) return;

        size_t block_shift = shift_bits / 64;
        size_t bit_shift = shift_bits % 64;
        uint64_t carry = 0, next_carry = 0;
        
        size_t dst;
        for (size_t i = 0; i < gx.data.size(); ++i) {
            dst = i + block_shift;
            if (dst >= data.size()) break;

            uint64_t word = gx.data[i];

            // If bit_shift is 0, 64-0 is 64, and shifting a 64-bit word right by 64 is undefined behavior
            next_carry = (bit_shift == 0) ? 0 : (word >> (64 - bit_shift));

            // data[dst] is equivalent to shifting the whole block to the same degree as the current data
            // word << bit_shift shifts the bits within the block to align with the current degree
            data[dst] ^= (word << bit_shift) | carry;
            carry = next_carry;
        }

        dst = gx.data.size() + block_shift;
        if (carry && dst < data.size()) {
            data[dst] ^= carry;
        }

        if (bitCount % 64 != 0 && !data.empty()) {
            uint64_t keep_mask = (1ULL << (bitCount % 64)) - 1ULL;
            data.back() &= keep_mask;
        }
    }
};

#endif // PACKEDBIT_H