#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <cstdint>
#include <string>
using namespace std;
#pragma once
#include "tools.h"



/*
dmir ram port map:
device_id
----- smi(cpu smi)
addr
data
flags
----- smi(ssd smi)
addr
data
flags
-----
device_flags_in
device_flags_out
*/
class ram{
    public:
        uint32_t start_addr;
        uint32_t requierd_regs = 10;
        uint32_t device_id = 1;
        uint32_t ticks_per_clock = 8;
        //vector<uint32_t> mem;
        ram(){
            this->start_addr = protocall::get_start_addr(this->requierd_regs);
            interface::init_device(this);
            cout << ("created ram instance starting at addr " + to_string(this->start_addr) + " and going " + to_string(this->requierd_regs)) << endl;
            this->init();
        }
        void clock(int loops){
            if(loops % this->ticks_per_clock == 0){
                print("clocked ram modual");
            }
        }
    private:    
        void init(){
            interface::mem->set_addr(this->start_addr,this->device_id);
        }

        
};


/*


0:
1:
2:
3:
4:
5:
6:
7:
8:
9:
10:


ram1:8_ports
usb:5_ports    

stdf:[
p1:-> device_id


]

*/