#include <iostream> 
#include <unordered_map>
#include <string> 
#include <tuple> 
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <algorithm> 
#include <random>    
#include <numeric>   
#include <cmath> 
#include <limits>
#include <cctype> 
#include <iomanip>
#include <chrono>

struct Timer {
    std::chrono::_V2::system_clock::time_point start, end; 
    std::chrono::duration<float> duration; 
    std::string function_name; 

    Timer(std::string name){
        function_name = name; 
        start = std::chrono::high_resolution_clock::now(); 
    }
    ~Timer(){
        end = std::chrono::high_resolution_clock::now(); 
        duration = end-start; 
        std::cout << function_name << "took: " << duration.count() * 1000 << "ms\n"; 
    }

}; 

// Generic file reading function
std::ifstream read_file(std::string filename){

    Timer timer = Timer("read_file"); 
    std::ifstream inputfile; 
    inputfile.open("./" + filename); 
    if (!inputfile.is_open()){
        throw std::runtime_error("File not found: " + filename); 
    }
    return inputfile; 
}

// Function to read quadgrams
std::tuple<std::unordered_map<std::string, double>, long long>
read_quadgrams(std::string filename){
    Timer timer = Timer("read_quad_grams"); 
    std::ifstream inputfile = read_file(filename); 
    std::string line; 
    std::string quad_gram; 
    long long quad_count; 
    long long total = 0; 

    std::unordered_map<std::string, double> quad_dictionary; 
    // Reserve space to avoid rehashing during load (approx 300k quadgrams in English)
    quad_dictionary.reserve(300000);

    while(getline(inputfile, line)){
        std::stringstream ss(line); 
        if (ss >> quad_gram >> quad_count){
            total += quad_count;  
            quad_dictionary[quad_gram] = quad_count; 
        }
    }
    inputfile.close();

    // Calculate log probabilities
    for (auto& [key, value] : quad_dictionary)
        value = std::log10(value / (double)total); 

    return {quad_dictionary, total}; 
}

// Function to read the ciphertext
std::string read_cipher(std::string filename){
    Timer timer = Timer("read_cipher"); 
    std::string cipher{}; 
    std::ifstream inputfile = read_file(filename); 
    
    char c;
    while(inputfile.get(c)){
        if (std::isalpha(c)) {
            cipher += std::toupper(c);
        }
    }
    inputfile.close(); 
    return cipher; 
}
 
// Function to create a random key 
std::unordered_map<char, char> random_mapping(){
    
    std::vector<char> plain(26); 
    std::iota(plain.begin(), plain.end(), 'A');

    std::vector<char> cipher = plain; 

    std::random_device rd;  
    std::mt19937 g(rd());
    std::shuffle(cipher.begin(), cipher.end(), g);

    std::unordered_map<char, char> key_map; 
    for (int i = 0; i < 26; ++i){
        key_map[cipher[i]] = plain[i]; 
    }
    return key_map; 
}

// function to computer log likelihood 
double log_likelihood(const std::unordered_map<std::string, double>& quad_dict, const std::string& text){
    double score = 0.0;
    const double penalty = -20.0; 
    size_t len = text.length();
    
    if (len < 4) return score;

    std::string quad(4, ' '); 
    
    auto end_iter = quad_dict.end();

    for (size_t i = 0; i < len - 3; ++i) {
        quad[0] = text[i];
        quad[1] = text[i+1];
        quad[2] = text[i+2];
        quad[3] = text[i+3];

        // Single lookup
        auto it = quad_dict.find(quad);
        if (it != end_iter) {
            score += it->second; 
        } else {
            score += penalty;
        }
    }
    return score;
}

// Hill Climbing Algorithm
void hill_climbing(int steps, 
                int restarts, 
                const std::string& cipher, 
                const std::unordered_map<std::string, double>& dict){

    Timer timer = Timer("hill climbing"); 
    std::unordered_map<char, char> best_global_mapping; 
    double best_global_score = -std::numeric_limits<double>::infinity();

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dis(0, 25);

    // Pre-allocate the decryption string once to avoid re-allocation
    std::string decryption = cipher; 

    std::cout << "Starting decryption on " << cipher.length() << " characters..." << std::endl;

    for (int i = 0; i < restarts; ++i){
        
        std::unordered_map<char, char> current_key = random_mapping();
        
        // Initial Decryption (Direct overwrite) 
        for (size_t k = 0; k < cipher.size(); ++k) {
            decryption[k] = current_key[cipher[k]];
        }
        
        double current_score = log_likelihood(dict, decryption);
        
        for (int j = 0; j < steps; ++j) {
            char a = 'A' + dis(g); 
            char b = 'A' + dis(g); 
            
            if (a == b) continue;

            // Swap in Map
            std::swap(current_key[a], current_key[b]);

            
            for (size_t k = 0; k < cipher.size(); ++k) {
                decryption[k] = current_key[cipher[k]];
            }
            
            double new_score = log_likelihood(dict, decryption);

            if (new_score > current_score) {
                current_score = new_score;
                if (current_score > best_global_score){
                    best_global_score = current_score;
                    best_global_mapping = current_key;
                }
            } 
            else {
                // Revert swap
                std::swap(current_key[a], current_key[b]); 
                
            }
        }
    }

    std::string final_decryption = "";
    for (char c : cipher) final_decryption += best_global_mapping[c];

    std::cout << "\n--- FINAL DECRYPTED TEXT ---\n" << std::endl;
    std::cout << final_decryption << std::endl;                 
}

int main(){
    // Fast IO
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    try {
        std::string ciphertext = read_cipher("cipher.txt"); 
        
        if (ciphertext.length() < 10) {
            std::cerr << "Error: Ciphertext is too short or empty." << std::endl;
            return 1;
        }

        auto [quad_dict, total_quad] = read_quadgrams("english_quadgrams.txt"); 
        std::cout << "Quadgrams loaded successfully." << std::endl; 

        int steps = 8000; 
        int restarts = 20; 
        
        hill_climbing(steps, restarts, ciphertext, quad_dict); 

        // --- STATS ---
        double total_permutations = 4.03291461126605635584e26; 
        double explored_keys = (double)steps * restarts;

        std::cout << "\n--- STATISTICS ---" << std::endl;
        std::cout << "Keys Explored     : " << std::scientific << explored_keys << std::endl;
        std::cout << "Keys Explored (2^x): 2^" << std::fixed << std::setprecision(2) << std::log2(explored_keys) 
              << " vs Total Space: 2^" << std::log2(total_permutations) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
    }

    return 0; 
}