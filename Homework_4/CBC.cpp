#include "DES.h"
#include <bitset>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// The blocks has been padded and is already a vector of binary string
// The key and IV are also binary strings

string CBC_encryption(vector<string> blocks, string key, string IV) {
  string encrypted;
    for (auto block: blocks){
      IV = encryption(XOR(IV, block, 64), key_gen(key));
      encrypted += IV;

    } 
  return encrypted;
}

string CBC_decryption(vector<string> blocks, string key, string IV) {
  string decrypted;
  string previous = IV; 
  for (auto block: blocks){
      decrypted += XOR(decryption(block, key_gen(key)), previous, 64);
      previous = block;
  } 
  
  return decrypted;
}