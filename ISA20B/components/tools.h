#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <functional>
using namespace std;
#pragma once



class Memory;
class ram;
class ssd;
namespace interface{

    Memory *mem;
    vector<function<void(int)>> devices;

    template<typename T>
    void init_device(T* instance) {
        // This creates the lambda for ANY class T that has a .clock(int) method
        devices.push_back([instance](int loops) {
            instance->clock(loops);
        });
    }

    void init(Memory *port_mem){
        print("initilazing devices");
        mem = port_mem;
        //initialize_devices();
    }
    void write_port(uint32_t addr,uint32_t value){
        mem->set_addr(addr,value);
    }
    uint32_t read_port(uint32_t addr){
            cout << "read from port " << addr << endl;
            return mem->get_addr(addr);
    }
    void clock(int loops){
        for(int i = 0 ; i < devices.size() ; i++){
            devices[i](loops);
        }
    }
};
namespace protocall{
    /*
    protocall:
    reg0:protocall number
    reg1:end of ptr alocation

    */
    uint32_t protocall_num = 1;
    uint32_t protocall_alocation = 2;
    uint32_t ptr_alocation = 15;
    void fill_alocation(uint32_t start_addr,uint32_t end_addr,uint32_t value = 0){
        for(uint32_t i = start_addr ; i < end_addr ; i++){
            interface::mem->set_addr(i,value);
        }
    }
    uint32_t get_start_addr(uint32_t requierd_regs) {
        uint32_t value = 0;
        // get open ptr_reg
        uint32_t addr = protocall_alocation + 1; 
        uint32_t current_val = 0;
        for(uint32_t i = protocall_alocation + 1; i < protocall_alocation + ptr_alocation; i++) {
            current_val = interface::mem->get_addr(i);
            //cout << "acsesing addr" << i << endl;
            if(current_val == 0) {
                addr = i;
                break;
            }
        }
        // get alocation
        // 1. Get the last used address
        uint32_t last_end = interface::mem->get_addr(addr - 1);
        uint32_t start_addr = last_end + 1;
        uint32_t end_addr = start_addr + (requierd_regs);
        interface::mem->set_addr(addr, end_addr);
        return start_addr;
    }
    void init(){
        interface::mem->set_addr(0,protocall_num);
        interface::mem->set_addr(0,ptr_alocation); // 15 for 15 alocarions
        fill_alocation(protocall_alocation,protocall_alocation+ptr_alocation);
    }

};
