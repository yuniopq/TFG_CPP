#ifndef GALOIS_FIELD_H
#define GALOIS_FIELD_H

#include <vector>
#include <cstdint>
class GaloisField{

private:
    
    int m;                  // Degree of the field
    int size;               // 2^m
    int primitive_poly;     // Primitive Polynomial

    static std::vector<uint16_t> exp_table;  // Antilog table
    static std::vector<uint16_t> log_table;  // Log table
    static int initialized_m; // Para saber si ya las calculamos
    
public:
    GaloisField()=default;
    GaloisField(int m=0, int primitive_poly=0);
    uint16_t add(uint16_t a, uint16_t b) const;
    uint16_t multiply(uint16_t a, uint16_t b) const;
    uint16_t divide(uint16_t a, uint16_t b) const;
    uint16_t inverse(uint16_t a) const;
    uint16_t power(uint16_t a, int e) const;


};

#endif