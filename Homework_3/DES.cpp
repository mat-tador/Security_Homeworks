#include "DES.h"
#include "permutation.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

string encryption(string plaintext, vector<string> keys) {

    // Initial permutation 
    std::string permuted = ""; 
    for (int i = 0; i < 64; ++i){
        permuted += plaintext[IP[i]-1];
    }

    // Loop 
    std::string left, right;    
    left = permuted.substr(0, 32); 
    right = permuted.substr(32, 32); 

    for (int i = 0; i<16; ++i){
        std::string old_right = right;
        
        right = XOR(pBox((sBox(XOR(expansion(old_right), keys[i], 48)))), left, 32); 
        left = old_right; 
    }

    // Final permutation
    std::string combined = right + left; 
    std::string final = ""; 
    for (int i = 0; i < 64; ++i){
        final += combined[FP[i]-1];
    }

    return final; 

}

string decryption(string encrypted, vector<string> keys) {

    // Initial permutation 
    std::string permuted = ""; 
    for (int i = 0; i < 64; ++i){
        permuted += encrypted[IP[i]-1];
    }

    // Loop 
    std::string left, right;    
    left = permuted.substr(0, 32); 
    right = permuted.substr(32, 32); 

    for (int i = 15; i>=0; --i){
        std::string old_right = right;
        
        right = XOR(pBox((sBox(XOR(expansion(old_right), keys[i], 48)))), left, 32); 
        left = old_right; 
    }

    // Final permutation
    std::string combined = right + left; 
    std::string final = ""; 
    for (int i = 0; i < 64; ++i){
        final += combined[FP[i]-1];
    }

    return final; 
}
