#ifndef PACKEDBIT_H
#define PACKEDBIT_H

class PackedBit{
public:
    std::vector<uint64_t> data;
    size_t bitCount;

    PackedBit(const std::vector<uint16_t> &original){
        bitCount = original.size();
        size_t blockCount = (bitCount + 63) / 64;
        data.resize(blockCount, 0);
        for (size_t i=0; i<bitCount; i++){
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
        vector<uint16_t> res(bitCount, 0);
        for(size_t i=0; i<bitCount; i++){
            res[i] = (data[i/64] >> (i%64)) & 1ULL;
        }
        return res;
    }

    std::vector<uint16_t> toVector(){
        std::vector<uint16_t> res(bitCount, 0);
        size_t fullBlocks = bitCount / 64;
        for(size_t block=0; block<fullBlocks; block++){
            uint64_t blockData = data[block];
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

    void xor(PackedBit &other){
        size_t minBlocks = std::min(data.size(), other.data.size());
        for(size_t i=0; i<minBlocks; i++){
            data[i] ^= other.data[i];
        }
    }

    




};

#endif // PACKEDBIT_H