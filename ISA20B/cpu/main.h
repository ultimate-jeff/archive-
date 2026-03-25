
#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <string>
#include <functional>
#include <nlohmann/json.hpp>
using namespace nlohmann;
#include "memory.h"
#include "global_utils.h"
#include "pu.h"
#pragma once

//compile command !! must run command befor runing :
// g++ main.cpp -o main -lsfml-graphics -lsfml-window -lsfml-system
// run command : ./main

using namespace std;
using file = std::ifstream;
Offset_mem offm;
Offset_mem offm_banks[8] = {
    Offset_mem(), Offset_mem(), Offset_mem(), Offset_mem(),
    Offset_mem(), Offset_mem(), Offset_mem(), Offset_mem()
};
uint32_t curent_offmb = 0;

uint8_t offset_map[32] = {

    /*0  NOP  */ 0b000,
    /*1       */ 0b100,
    /*2       */ 0b110,
    /*3       */ 0b111,
    /*4       */ 0b110,
    /*5       */ 0b110,
    /*6       */ 0b100,
    /*7       */ 0b100,
    /*8       */ 0b000,
    /*9       */ 0b110,
    /*10      */ 0b110,
    /*11      */ 0b110,
    /*12      */ 0b110,
    /*13      */ 0b100,
    /*14      */ 0b100,
    /*15      */ 0b100,
    /*16      */ 0b110,
    /*17      */ 0b000,
    /*18      */ 0b010,
    /*19      */ 0b100,
    /*20      */ 0b100,
    /*21      */ 0b100,
    /*22      */ 0b000,
    /*23      */ 0b000,
    /*24      */ 0b000,
    /*25      */ 0b000,
    /*26      */ 0b000,
    /*27      */ 0b000,
    /*28      */ 0b000,
    /*29      */ 0b000,
    /*30      */ 0b110,
    /*31      */ 0b110
};

class Core;
extern Core cores[8];
class Core{
public:
    uint32_t curent_offmb = 0;
    uint32_t core_id;
    static uint32_t instances;
    uint32_t core_addr;
    Memory mem;
    PU pu;
    PC pc;
    uint32_t curent_raw_inst = 0;
    bool running = true;
    bool do_jmp = true;
    uint32_t stall_ticks = 0;
    using Instr = void (Core::*)(uint32_t[3]);
    vector<Instr> op_methods = {
        &Core::HULT,
        &Core::stall,
        &Core::LR,
        &Core::push,
        &Core::pull,
        &Core::push_ptr,
        &Core::pull_ptr,
        &Core::add,
        &Core::sub,
        &Core::AND,
        &Core::NAND,
        &Core::OR,
        &Core::XOR,
        &Core::MOVE,
        &Core::cmp,
        &Core::jmp,
        &Core::jmp_ptr,
        &Core::stack,
        &Core::interupt,
        &Core::ADI,
        &Core::SDI,
        &Core::shift_u,
        &Core::shift_d,
        &Core::cstate,
        &Core::call,
        &Core::ret,
        &Core::ld_ptr,
        &Core::ldoffm_ptr,
        &Core::LD_off,
        &Core::SoffmB,
        &Core::push_c,
        &Core::pull_c
    };
    uint32_t spliting_map[32] = {0,1,2,2,2,2,2,3,3,3,3,3,3,2,3,1,1,0,3,2,2,3,3,2,1,0,2,3,2,2,0,0};
    
    Core(){
        this->core_id = Core::instances;
        Core::instances++;
        this->core_addr = offm_banks[this->curent_offmb].get_core_addir(this->core_id);
    };
    void clock(){
        if (this->stall_ticks > 0){
            this->stall_ticks--;
            return;
        }
        this->pc.clock();
    }
    uint32_t get_curent_inst(uint32_t addr){
        this->curent_raw_inst = this->mem.get_addr(addr+offm_banks[this->curent_offmb].get_addr(3,this->core_addr));
        return this->curent_raw_inst;
    }
    uint32_t add_offset(uint32_t op,uint32_t op_index,uint32_t opcode){
        uint32_t offset_bit_map = offset_map[opcode];
        bool add_offset = (offset_bit_map >> (2 - op_index)) & 1;
        if(add_offset){
            op += offm_banks[this->curent_offmb].get_addr(op_index,this->core_addr);
        }
        return op;
    }
    void exec_inst(){
        uint32_t op[3] = {0,0,0};
        this->get_curent_inst(this->pc.counter);
        uint32_t opcode = get_bit_section(this->curent_raw_inst,15,5);
        uint32_t format = this->spliting_map[opcode];
        //cout << "exacuting op " << opcode << " in core " << this->core_id <<endl;
        print("exacuting op " + to_string(opcode) + " in core " + to_string(this->core_id));
        switch(format){
            case 0:
                break;
            case 1:
                op[0] = get_bit_section(this->curent_raw_inst,0,15);
                op[0] = this->add_offset(op[0],0,opcode);// add offset
                break;

            case 2:
                /*
                op[0] = get_bit_section(this->curent_raw_inst, 5, 5);   // Reads bits 5-9
                op[1] = get_bit_section(this->curent_raw_inst, 10, 10);
                */
                op[0] = get_bit_section(this->curent_raw_inst, 10, 5); // Register (Bits 10-14)
                op[1] = get_bit_section(this->curent_raw_inst, 0, 10);
                op[0] = this->add_offset(op[0],0,opcode);// add offset
                op[1] = this->add_offset(op[1],1,opcode);// add offset
                break;

            case 3:
                op[0] = get_bit_section(this->curent_raw_inst,10,5);
                op[1] = get_bit_section(this->curent_raw_inst,5,5);
                op[2] = get_bit_section(this->curent_raw_inst,0,5);
                op[0] = this->add_offset(op[0],0,opcode);// add offset
                op[1] = this->add_offset(op[1],1,opcode);// add offset
                op[2] = this->add_offset(op[2],2,opcode);// add offset
                break;
        }

        Instr func = op_methods[opcode];
        (this->*func)(op);
        this->clock();
    }

    void HULT(uint32_t op[3]){
        this->running = false;
    }
    void stall(uint32_t op[3]){ // offsetM:000
        this->stall_ticks = op[0];
    }
    //void LR(uint32_t op[3]){ // reg , data : offsetM:100
    //    this->mem.set_addr(op[0],op[1]);
    //}
    void LR(uint32_t op[3]){ // reg , data
        uint32_t data = mask(op[1], b10_mask);
        uint32_t flags = this->pu.alu.gen_flags(data, data, 0); // gen flags
        uint32_t packed = (flags << 10) | data;
        this->mem.set_addr(op[0], packed);
    }
    void push(uint32_t op[3]){ // reg , data : offsetM:100
        uint32_t value = this->mem.get_addr(op[0]);
        this->mem.set_addr(op[1],value);
    }
    void pull(uint32_t op[3]){ // reg , data : offsetM:100
        uint32_t value = this->mem.get_addr(op[1]);
        this->mem.set_addr(op[0],value);
    }
    void push_ptr(uint32_t op[3]){ // reg , reg_ptr : offsetM:110
        op[1] = this->ptr_ld(op[1]);
        uint32_t value = this->mem.get_addr(op[0]);
        this->mem.set_addr(op[1],value);
    }
    void pull_ptr(uint32_t op[3]){ // reg , reg_ptr : offsetM:110
        op[1] = this->ptr_ld(op[1]);
        uint32_t value = this->mem.get_addr(op[1]);
        this->mem.set_addr(op[0],value);
    }
    void add(uint32_t op[3]){ // regA , regB , regC : offsetM:111
        this->mem.set_addr(op[2],this->pu.ADD(mem.get_addr(op[0]),mem.get_addr(op[1])));
    }
    void sub(uint32_t op[3]){ // regA , regB , regC : offsetM:111
        this->mem.set_addr(op[2],this->pu.SUB(mem.get_addr(op[0]),mem.get_addr(op[1])));
    }
    void AND(uint32_t op[3]){ // regA , regB , regC : offsetM:111
        this->mem.set_addr(op[2],this->pu.AND(mem.get_addr(op[0]),mem.get_addr(op[1])));
    }
    void NAND(uint32_t op[3]){ // regA , regB , regC : offsetM:111
        this->mem.set_addr(op[2],this->pu.NAND(mem.get_addr(op[0]),mem.get_addr(op[1])));
    }
    void OR(uint32_t op[3]){ // regA , regB , regC : offsetM:111
        this->mem.set_addr(op[2],this->pu.OR(mem.get_addr(op[0]),mem.get_addr(op[1])));
    }
    void XOR(uint32_t op[3]){ // regA , regB , regC : offsetM:111
        this->mem.set_addr(op[2],this->pu.XOR(mem.get_addr(op[0]),mem.get_addr(op[1])));
    }
    void MOVE(uint32_t op[3]){ // : offsetM:111
        uint32_t src_reg = ptr_ld(op[0]);
        uint32_t dst_reg = ptr_ld(op[1]);
        uint32_t value = mem.get_addr(src_reg);
        mem.set_addr(dst_reg, value);
    }
    void cmp(uint32_t op[3]){ // reg , flags , invert_mask : offsetM:100
        uint32_t reg = this->mem.get_addr(op[0]);
        uint32_t raw_flags = get_bit_section(reg,10,5);
        uint32_t cmp_flags = mask(op[1], b5_mask);
        raw_flags ^= mask(op[2], b5_mask); // add invert mask
        print("set do jmp to : " + to_string(this->do_jmp) + " and used cmp flags " + to_string(raw_flags) + " in reg");
        this->do_jmp = (raw_flags & cmp_flags) != 0;// == cmp_flags;     != 0;
    }
    void jmp(uint32_t op[3]){ // address : offsetM:000
        if(this->do_jmp){
            this->pc.jmp(op[0],this->do_jmp,false);
            print("jumped to " + to_string(op[0]));
        }
    }
    void jmp_ptr(uint32_t op[3]){ // addr_ptr : offsetM:111
        uint32_t value = this->ptr_ld(op[0]);
        if(this->do_jmp){
            this->pc.jmp(value,this->do_jmp,false);
            print("jumped to " + to_string(value));
        }
    }
    void stack(uint32_t op[3]){ // null
        
    }
    void interupt(uint32_t op[3]){ // core_id, addr_ptr :offsetm:010
        uint32_t sub_op[3] = {this->mem.get_addr(op[1]),0,0};
        cores[mask(op[0],b5_mask)].call(sub_op);
    }
    
    void ADI(uint32_t op[3]){ // reg , data  : offsetM:100
        this->mem.set_addr(op[0],this->pu.ADD(this->mem.get_addr(op[0]),op[1]));
    }
    void SDI(uint32_t op[3]){ // reg , data  : offsetM:100
        this->mem.set_addr(op[0],this->pu.SUB(this->mem.get_addr(op[0]),op[1]));
    }
    void shift_u(uint32_t op[3]){ // reg_a/reg , reg_b/amount , reg_c/output : offsetM:111
        this->mem.set_addr(op[2],this->mem.get_addr(op[0]) << this->mem.get_addr(op[1]));
    }
    void shift_d(uint32_t op[3]){ // reg_a , reg_b , reg_c : offsetM:111
        this->mem.set_addr(op[2],this->mem.get_addr(op[0]) >> this->mem.get_addr(op[1]));
    }
    void cstate(uint32_t op[3]){ // core_id ,start/stop : offsetM:100
        //cout << "this is not filed in" << endl;
        op[0] = mask(op[0],b3_mask);
        cores[op[0]].running = (op[1] > 0);
    }
    void call(uint32_t op[3]){ // addr : offsetM:000
        this->pc.jmp(op[0],true,true);
    }
    void ret(uint32_t op[3]){ // : offsetM:000
        this->pc.ret();
    }
    void ld_ptr(uint32_t op[3]){ // t_reg , reg : offsetM:111
        uint32_t addr = this->mem.get_addr(op[1]); // get value of reg
        uint32_t value = this->mem.get_addr(addr);
        this->mem.set_addr(op[0],value);
    }
    void ldoffm_ptr(uint32_t op[3]){ // offset_reg_ptr , ptr_reg  : offsetM:111
        offm_banks[this->curent_offmb].set_addr(this->ptr_ld(op[0]),this->core_addr,this->ptr_ld(op[1]));
    }
    void LD_off(uint32_t op[3]){ // offset_reg , data : offsetM:100
        offm_banks[this->curent_offmb].set_addr(op[0],this->core_addr,op[1]);
    }
    void SoffmB(uint32_t op[3]){ //set offset memory bank : offset_reg_bank , core_id
        uint32_t value = this->mem.get_addr(op[0]);
        this->curent_offmb = mask(value,b5_mask);
    }
    void push_c(uint32_t op[3]){ // reg, t_reg , core_id
        cores[mask(op[2],b3_mask)].mem.set_addr(op[1],this->mem.get_addr(op[0]));
    }
    void pull_c(uint32_t op[3]){// reg, t_reg , core_id
        this->mem.set_addr(op[1],cores[mask(op[2],b3_mask)].mem.get_addr(op[0]));
    }
private:
    uint32_t ptr_ld(uint32_t addr){
        return this->mem.get_addr(addr);
    }
};

Core cores[8] = {
    Core(),
    Core(),
    Core(),
    Core(),
    Core(),
    Core(),
    Core(),
    Core()
};

uint32_t Core::instances = 0;
int CLOCK(int active_cores){
    int active_coress = 0;
    for(int i = 0; i < 8 ; i++){
        if(cores[i].running){
            //cout << "exacuting core: " << i << endl;
            cores[i].exec_inst();
            active_coress++;
        }
    }
    return active_coress;
}
void start_cpu(){
    int loops = 0;
    int active_cores = 1;
    while(active_cores >= 1){
        loops++;
        //cout << "on loop " << loops << endl;
        if (loops % 50 == 0){
            cout << "on loop " << loops << endl;
        }
        active_cores = CLOCK(active_cores);
    }
    cout << "program ended" << endl;
}

