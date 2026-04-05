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
    inline __attribute__((always_inline))  uint32_t get_abs_addr(uint32_t addr){
        return this->mem[(addr & b5_mask)];
    }
    inline __attribute__((always_inline))  void set_abs_addr(uint32_t addr, uint32_t value){
        this->mem[(addr & b5_mask)] = value;
    }
    inline __attribute__((always_inline))  uint32_t  get_addr(uint32_t offset ,uint32_t core_addir){
        return this->mem[(offset & b5_mask) + (core_addir & b5_mask)];
    }
    inline __attribute__((always_inline))  void set_addr(uint32_t offset ,uint32_t core_addir, uint32_t value){
        this->mem[(offset & b5_mask) + (core_addir & b5_mask) ] = value;
    }
    inline __attribute__((always_inline))  uint32_t gen_core_addr(uint32_t offset, uint32_t core_id){
        offset = offset & b5_mask;
        core_id = core_id & b4_mask;
        return offset + (core_id * offsets_per_core);
    }
    inline __attribute__((always_inline))  uint32_t get_core_addir(uint32_t core_id){
        core_id = core_id & b5_mask;
        return core_id * offsets_per_core;
    }
};

class Memory {
public:
    //vector<vector<bool>> mem;
    static const uint32_t Bmask = b20_mask;
    static const uint32_t addr_mask = b12_mask;
    vector<uint32_t> mem;
    //uint32_t* mem;
    uint32_t core_id;
    int instances = 0;
    Memory(uint32_t core_id = -1): mem(4096,0){
        //mem = new uint32_t[4096]();

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
        return value & this->addr_mask;
    };
    inline __attribute__((always_inline))uint32_t get_addr(uint32_t addr){
        return this->mem[addr & this->addr_mask];
    };
    inline __attribute__((always_inline))void set_addr(uint32_t addr, uint32_t value){
        this->mem[addr & this->addr_mask] = value & b20_mask;
    };
    // i love c++
    inline __attribute__((always_inline))void load_reg(uint32_t addr, uint32_t value, uint32_t offsets){
        this->mem[(addr+offsets) & this->addr_mask] = value & b20_mask;//this->set_addr(addr, value);
    };
    inline __attribute__((always_inline))uint32_t read_reg(uint32_t addr, uint32_t offsets){
        return this->mem[(addr+offsets) & this->addr_mask];//return this->get_addr(addr);
    };
};