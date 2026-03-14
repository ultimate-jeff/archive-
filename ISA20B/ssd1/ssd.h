#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
//#include <nlohmann/json.hpp>
//using namespace nlohmann;
using namespace std;
#pragma once

class Memory;
namespace lmad{
    vector<uint32_t> readTextUints(const string& path) {
        ifstream file(path);
        vector<std::uint32_t> values;
        if (!file.is_open()) {
            file.close();
            return values; // Or handle error
        }
        std::uint32_t val;
        while (file >> val) {
            values.push_back(val);
        }
        file.close();
        return values;
    }
    class ssd{
        Memory mem;
        int cpt = 40;
        int addr_size = 15;
        uint32_t size_mask = 32768;
        uint32_t port_map_num = 1;
        public:
            ssd(Memory mem_u){
                this->mem = mem_u;
            }
    };
}