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




/*
ssd port map:
device_id
smi_ptr
------ smi
addr
data
flags
------
addr_start
num_of_pages
device_flags_in
device_flags_out
*/



class ssd{
    public:
        uint32_t start_addr;
        uint32_t requierd_regs = 10;
        ssd(){
            this->start_addr = protocall::get_start_addr(this->requierd_regs);
            interface::init_device(this);
            cout << ("created ssd instance starting at addr " + to_string(this->start_addr) + " and going " + to_string(this->requierd_regs)) << endl;
        }
        void clock(int loops){
            
        }
};
