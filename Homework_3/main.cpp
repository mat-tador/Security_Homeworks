#include "DES.h"
#include <bitset>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main() {
  // do not write code in this file. Only write code in the DES.cpp
  string encryption_flag;
  cout << "Enter e or d to encrypt or decrypt: ";
  cin >> encryption_flag;
  string plaintext = hexToBinary("453001EDED553001");
  string key = hexToBinary("02FFA374EB55F57B");
  vector<string> sub_keys = key_gen(key);
  if (encryption_flag[0] == 'e') {
    string encrypted = encryption(plaintext, sub_keys);
    cout << encrypted << endl;
  } else if (encryption_flag[0] == 'd') {
    string encrypted = encryption(plaintext, sub_keys);
    string decrypted = decryption(encrypted, sub_keys);
    cout << decrypted << endl;
  }
}
