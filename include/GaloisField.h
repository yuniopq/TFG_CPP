#ifndef GALOIS_FIELD_H
#define GALOIS_FIELD_H

#include <vector>

using namespace std;

class GaloisField{

private:
    
    int m;                  // Degree of the field
    int size;               // 2^m
    int primitive_poly;     // Primitive Polynomial

    vector<int> exp_table;  // Antilog table
    vector<int> log_table;  // Log table
    
public:
    GaloisField(int m, int prymitive_poly);
    int add(int a, int b);
    int multiply(int a, int b);
    int divide(int a, int b);
    int inverse(int a);
    int power(int a, int e);


};

#endif