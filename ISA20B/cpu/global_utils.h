#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono> // for time
#pragma once

using namespace std;

string print_que;
const int mem_size = 4096;
const int mem_addr_size = 12;
const int reg_size = 20;
const int alu_data_size = 10;
const int flag_size = 5;
const int offset_mem_addr_size = 5;
const int offsets_per_core = 4;
const int core_addir_size = 4;
const uint32_t b20_mask = 0xFFFFF;
const uint32_t b12_mask = 0xFFF;
const uint32_t b10_mask = 0x3FF;
const uint32_t b5_mask = 0x1F;
const uint32_t b1_mask = 0x1;
const uint32_t b4_mask = 0xF;
const uint32_t b15_mask = 0x7FFF;
const uint32_t b8_mask = 0xFF;
const uint32_t b3_mask = 0x7;

uint32_t sine_mask(uint32_t mask){
    return (mask + 1) >> 1;
};
uint32_t mask(uint32_t value, uint32_t mask){
    return value & mask;
};
uint32_t get_bit_section(uint32_t value, uint32_t start_bit, uint32_t num_bits){
    uint32_t m = (1 << num_bits) - 1;
    return (value >> start_bit) & m;
};
int32_t conv_to_int(uint32_t value, uint32_t mask){
    value &= mask;
    uint32_t sign = sine_mask(mask);
    uint32_t width = __builtin_popcount(mask); // GCC/Clang
    if (value & sign)
        return (int32_t)(value - (1u << width));
    return (int32_t)value;
}
void print(string msg){
    print_que += (msg + "\n");
}
void cout_print_que(){
    cout << print_que;
    print_que = ""; 
}

class Call_stack{
    vector<uint32_t> stack;//[b8_mask];
public:
    Call_stack(){
        this->fill_mem(0);
    }
    void fill_mem(uint32_t value=0){
        value = mask(value,b12_mask);
        for(int i = 0; i < b8_mask ; i++){
            this->stack.push_back(value);
        }
    }
    /*
    uint32_t get_top(bool pop = true){
        uint32_t value;
        if(pop){
            value = this->stack.front();
            this->stack.erase(this->stack.begin());
        }
        else {
            value = this->stack.front();
        }
        return value;
    }
    void call(uint32_t addr){
        addr = mask(addr,b12_mask);
        this->stack.push_back(addr);
    }
    */
   uint32_t get_top(bool pop = true) {
        if (stack.empty()) return 0;
        uint32_t value = stack.back(); // Use back() for LIFO
        if (pop) stack.pop_back();
        return value;
    }

    void call(uint32_t return_addr) {
        stack.push_back(mask(return_addr, b12_mask));
    }

};

class PC{
public:
    uint32_t counter = 0;
    Call_stack call_stack;
    uint32_t flg_sum = 0;
    bool ret_on_clock = false;

    PC(){};
    void on_clock(){
        if(this->ret_on_clock){
            this->ret();
        }
    }
    void clock(){
        this->counter++;
        this->counter = mask(this->counter,b12_mask);
        this->on_clock();
    };
    void jmp(uint32_t addr, bool do_jmp, bool do_call = false) {
        if (do_jmp) {
            if (do_call) {
                // Push the NEXT instruction address as the return point
                this->call_stack.call(this->counter); 
            }
            this->counter = mask(addr, b12_mask);
        }
    }
    void ret() {
        this->counter = this->call_stack.get_top(true);
    }
};

class Timer { // i made this class with ai for testing
public:
    std::chrono::time_point<std::chrono::steady_clock> start_tp;

    void start_time() {
        start_tp = std::chrono::steady_clock::now();
    }
    double get_time() {
        auto end_tp = std::chrono::steady_clock::now();
        std::chrono::duration<double,milli> elapsed = end_tp - start_tp;
        return elapsed.count();
    }
    void print_time() {
        std::cout << "Elapsed time: " << std::fixed << std::setprecision(3) 
                  << get_time() << "s" << std::endl;
    }
};

