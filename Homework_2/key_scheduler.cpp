#include "key_utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <algorithm>
using namespace std;

/*
1) Performs a left rotation on a range of elements.
    Specifically, std::rotate swaps the elements in the range 
    [first, last) in such a way that the elements in [first, middle)
    are placed after the elements in [middle, last) while the orders 
    of the elements in both ranges are preserved.
*/
string shift_left(string sub_key){
    
    std::rotate(sub_key.begin(), sub_key.begin() + 1, sub_key.end()); 
    return sub_key;
} 

vector<string> key_gen(string key){
    vector<string> sub_keys;
    //write code from here
    // split left and right 
    // Compress

    string key_56 = "";
    for (int i=0; i < 56 ;++i){
        key_56 += key[PC1[i]-1];         
    } 

    string C, D; 
    C = key_56.substr(0, 28); 
    D = key_56.substr(28, 28); 
     

    for (int i =0; i < 16; ++i){

        switch (i)
        {
        case 0:
        case 1: 
        case 8:
        case 15:
            C = shift_left(C); 
            D = shift_left(D);
            break; 
        
        default:
            C = shift_left(C); 
            D = shift_left(D);
            C = shift_left(C); 
            D = shift_left(D);
            break;
        }
        
        string combined = C + D; 
        string key_comb = ""; 
        for (int j = 0; j < 48; ++j){
            key_comb += combined[PC2[j]-1]; 
        }

        sub_keys.push_back(key_comb); 
    }

    return sub_keys;
}
