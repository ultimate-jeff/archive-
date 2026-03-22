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
#include "tools.h"

class usb{
    public:
    uint32_t start_addr;
    uint32_t requierd_regs = 10;
    usb(){

    }
    void clock(int loops){

    }
};