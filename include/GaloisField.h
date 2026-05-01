#ifndef GALOIS_FIELD_H
#define GALOIS_FIELD_H

#include <vector>

class GaloisField{

private:
    
    int m;                  // Degree of the field
    int size;               // 2^m
    int primitive_poly;     // Primitive Polynomial

    std::vector<int> exp_table;  // Antilog table
    std::vector<int> log_table;  // Log table
    
public:
    GaloisField(int m, int primitive_poly);
    int add(int a, int b) const;
    int multiply(int a, int b) const;
    int divide(int a, int b) const;
    int inverse(int a) const;
    int power(int a, int e) const;


};

#endif