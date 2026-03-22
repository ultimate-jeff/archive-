#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <cstdint>
#include <string>
using namespace std;
#pragma once
#include "tools.h"

class Rmem{
    public:
    vector<uint32_t> mem;
    Rmem() : mem(1048576,0){

    }
    void fill(uint32_t value = 0){
        for(int i = 0 ; i < this->mem.size() ; i++){
            this->mem[i] = value;
        }
    }
    void set_addr(uint32_t addr , uint32_t value){
        if(addr <= this->mem.size()){
            this->mem[addr] = value;
        }
    }
    uint32_t get_addr(uint32_t addr){
        if(addr <= this->mem.size())
            return this->mem[addr];
        else{
            return 0;
        }
    }
    void move(uint32_t addr1,uint32_t addr2){
        this->set_addr(addr2,this->get_addr(addr1));
    }
};
/*
dmir ram port map:
=================
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
=================
*/
class ram{
    public:
        uint32_t start_addr;
        uint32_t requierd_regs = 10;
        uint32_t device_id = 1;
        uint32_t ticks_per_clock = 8;
        Rmem mem;
        ram(){
            this->start_addr = protocall::get_start_addr(this->requierd_regs);
            interface::init_device(this);
            cout << ("created ram instance starting at addr " + to_string(this->start_addr) + " and going " + to_string(this->requierd_regs)) << endl;
            this->init();
        }
        void clock(int loops) {
            if (loops % this->ticks_per_clock == 0) {
                print("ram modual clocked");
                // We pass the absolute address of the first register of each channel
                // Channel 1 (CPU): start_addr + 1, + 2, + 3
                this->smi_chanle(this->start_addr + 1); 
                
                // Channel 2 (SSD): start_addr + 4, + 5, + 6
                this->smi_chanle(this->start_addr + 4);
            }
        }

    private:    
        void init() {
            interface::mem->set_addr(this->start_addr, this->device_id);
        }

        uint32_t smi(uint32_t addr, uint32_t data, uint32_t flags) {
            if (flags == 1) { // Read
                return this->mem.get_addr(addr);
            }
            else if (flags == 2) { // Write
                this->mem.set_addr(addr, data);
                return 1; 
            }
            else if (flags == 3) { // Move (Copy addr1 to addr2)
                this->mem.move(addr, data);
                return 1;
            }
            return 0;
        }

        void smi_chanle(uint32_t base_reg) {
            // 1. Read the registers from main memory
            uint32_t r_addr  = interface::mem->get_addr(base_reg);     // offset +0
            uint32_t r_data  = interface::mem->get_addr(base_reg + 1); // offset +1
            uint32_t r_flags = interface::mem->get_addr(base_reg + 2); // offset +2

            // 2. Only execute if a command (flag) is actually present
            if (r_flags != 0) {
                uint32_t result = this->smi(r_addr, r_data, r_flags);

                // 3. Only update the data register if it's a READ (flag 1)
                // This prevents the '1' success code from overwriting data on Writes
                if (r_flags == 1) {
                    interface::mem->set_addr(base_reg + 1, result);
                }

                // 4. Clear the flag to signal the operation is complete
                interface::mem->set_addr(base_reg + 2, 0);
                
                print("RAM: Channel at " + to_string(base_reg) + " processed flag " + to_string(r_flags));
            }
        }

};