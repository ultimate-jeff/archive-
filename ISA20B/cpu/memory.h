#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#pragma once
#include "global_utils.h"

using namespace std;
class Offset_mem{
public:
    uint32_t mem[offset_mem_addr_size];
    Offset_mem(){
        this->fill_mem();
    }
    void fill_mem(uint32_t value = 0){
        for (int i = 0; i < offset_mem_addr_size; i++){
            this->mem[i] = value;
        }
    }
    uint32_t get_abs_addr(uint32_t addr){
        addr = mask(addr, b5_mask);
        return this->mem[addr];
    }
    void set_abs_addr(uint32_t addr, uint32_t value){
        addr = mask(addr, b5_mask);
        this->mem[addr] = value;
    }
    uint32_t  get_addr(uint32_t offset ,uint32_t core_addir){
        offset = mask(offset, b5_mask);
        core_addir = mask(core_addir, b5_mask);
        return this->mem[offset + core_addir ];
    }
    void set_addr(uint32_t offset ,uint32_t core_addir, uint32_t value){
        offset = mask(offset, b5_mask);
        core_addir = mask(core_addir, b5_mask);
        this->mem[offset + core_addir ] = value;
    }
    uint32_t gen_core_addr(uint32_t offset, uint32_t core_id){
        offset = mask(offset, b5_mask);
        core_id = mask(core_id, b4_mask);
        return offset + (core_id * offsets_per_core);
    }
    uint32_t get_core_addir(uint32_t core_id){
        core_id = mask(core_id, b5_mask);
        return core_id * offsets_per_core;
    }
};

class Memory {
public:
    //vector<vector<bool>> mem;
    static const uint32_t Bmask = b20_mask;
    static const uint32_t Amask = b12_mask;
    uint32_t mem[4096];
    uint32_t core_id;
    int instances = 0;
    Memory(uint32_t core_id = -1){
        if (core_id == -1){
            this->core_id = Memory::instances;
        } else {
            this->core_id = core_id;
        }
        Memory::instances++;
        this->fill_mem();
    };
    void fill_mem(uint32_t value = 0){
        for (int i = 0; i < mem_size; i++){
            this->mem[i] = value;
        }
    };
    uint32_t mask(uint32_t value){
        // creates the 20 bit value
        return value & this->Bmask;
    };
    uint32_t mask_addr(uint32_t value){
        // creates the 12 bit value
        return value & this->Amask;
    };
    uint32_t get_addr(uint32_t addr){
        addr = this->mask_addr(addr);
        return this->mem[addr];
    };
    void set_addr(uint32_t addr, uint32_t value){
        addr = this->mask_addr(addr);
        this->mem[addr] = this->mask(value);
    };
    // i love c++
    void load_reg(uint32_t addr, uint32_t value, uint32_t offsets){
        addr += offsets;
        this->set_addr(addr, value);
    };
    uint32_t read_reg(uint32_t addr, uint32_t offsets){
        addr += offsets;
        return this->get_addr(addr);
    };
};