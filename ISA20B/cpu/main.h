
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
    /*16      */ 0b100,
    /*17      */ 0b111,
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
struct log_entry{
    uint32_t pc;
    uint32_t opcode;
    uint32_t code_id;
};

class Core;
extern Core cores[8];
class Core{
public:
    uint32_t curent_offmb = 0;
    uint32_t core_id;
    uint32_t core_addr;
    uint32_t curent_raw_inst = 0;
    uint32_t stall_ticks = 0;
    static uint32_t instances;
    Memory mem;
    PU pu;
    PC pc;
    bool running = true;
    bool do_jmp = true;
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
    uint32_t spliting_map[32] = {0,1,2,2,2,2,2,3,3,3,3,3,3,2,3,1,1,1,3,2,2,3,3,2,1,0,2,3,2,2,0,0};
    
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
        print("exacuting op " + to_string(opcode) + " in core " + to_string(this->core_id) + " at address " + to_string(this->pc.counter));
        switch(format){
            case 0:
                break;
            case 1:
                this->decode_format_1(op,this->curent_raw_inst,opcode,this);
                break;
            case 2:
                this->decode_format_2(op,this->curent_raw_inst,opcode,this);
                break;
            case 3:
                this->decode_format_3(op,this->curent_raw_inst,opcode,this);
                break;
        }

        //Instr func = op_methods[opcode];
        //(this->*func)(op);
        switch (opcode)
        {
        case 0:this->HULT(op);break;
        case 1:this->stall(op);break;
        case 2:this->LR(op);break;
        case 3:this->push(op);break;
        case 4:this->pull(op);break;
        case 5:this->pull_ptr(op);break;
        case 6:this->push_ptr(op);break;
        case 7:this->add(op);break;
        case 8:this->sub(op);break;
        case 9:this->AND(op);break;
        case 10:this->NAND(op);break;
        case 11:this->OR(op);break;
        case 12:this->XOR(op);break;
        case 13:this->MOVE(op);break;
        case 14:this->cmp(op);break;
        case 15:this->jmp(op);break;
        case 16:this->jmp_ptr(op);break;
        case 17:this->stack(op);break;
        case 18:this->interupt(op);break;
        case 19:this->ADI(op);break;
        case 20:this->SDI(op);break;
        case 21:this->shift_u(op);break;
        case 22:this->shift_d(op);break;
        case 23:this->cstate(op);break;
        case 24:this->call(op);break;
        case 25:this->ret(op);break;
        case 26:this->ld_ptr(op);break;
        case 27:this->ldoffm_ptr(op);break;
        case 28:this->LD_off(op);break;
        case 29:this->SoffmB(op);break;
        case 30:this->push_c(op);break;
        case 31:this->pull_c(op);break;
        default:this->HULT(op);break;
        }
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
        uint32_t flags = 0;
        print("loading reg " + to_string(op[0]) + " with " + to_string(data) + " and flags " + to_string(flags));
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
        uint32_t value = mem.get_addr(ptr_ld(op[0]));
        mem.set_addr(ptr_ld(op[1]), value);
    }
    void cmp(uint32_t op[3]){ // reg , flags , invert_mask : offsetM:100
        //uint32_t reg = this->mem.get_addr(op[0]);
        uint32_t raw_flags = get_bit_section(this->mem.get_addr(op[0]),10,5);
        uint32_t cmp_flags = (op[1] & b5_mask);
        raw_flags ^= (op[2] & b5_mask); // add invert mask
        this->do_jmp = (raw_flags & cmp_flags) == cmp_flags;// == cmp_flags;     != 0;   >= 0;
        print("set do_jmp to : " + to_string(this->do_jmp) + " and regester flags where " + to_string(raw_flags) + " and cmp flags where " + to_string(cmp_flags));
    }
    void jmp(uint32_t op[3]){ // address : offsetM:000
        if(this->do_jmp){
            this->pc.jmp(op[0],this->do_jmp,false); // added -1 to ensure it jmp's to corect addr and this might have probs at addr 0
            print("jumped to " + to_string(op[0]));
        }
    }
    void jmp_ptr(uint32_t op[3]){ // addr_ptr , flags(jmp/call): offsetM:101
        uint32_t value = this->ptr_ld(op[0]);
        if(this->do_jmp){
            this->pc.jmp(value,this->do_jmp,false);
            print("jumped to " + to_string(value));
        }
    }
    void stack(uint32_t op[3]){ // null
        print("regester " + to_string(op[0]) + " has value " + to_string(mask(this->mem.get_addr(op[0]),b10_mask)));
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
        //uint32_t addr = this->mem.get_addr(op[1]); // get value of reg
        uint32_t value = this->mem.get_addr(this->mem.get_addr(op[1]));
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
    inline __attribute__((always_inline)) void decode_format_1(uint32_t * op, uint32_t curent_raw_inst, uint32_t opcode, Core* cpu){
        op[0] = cpu->add_offset(get_bit_section(curent_raw_inst,0,15),0,opcode);// add offset
    }
    inline __attribute__((always_inline)) void decode_format_2(uint32_t * op, uint32_t curent_raw_inst, uint32_t opcode, Core* cpu){
        //op[0] = get_bit_section(curent_raw_inst, 10, 5); // Register (Bits 10-14)
        //op[1] = get_bit_section(curent_raw_inst, 0, 10);
        op[0] = cpu->add_offset(get_bit_section(curent_raw_inst, 10, 5),0,opcode);// add offset
        op[1] = cpu->add_offset(get_bit_section(curent_raw_inst, 0, 10),1,opcode);// add offset
    }
    inline __attribute__((always_inline)) void decode_format_3(uint32_t * op, uint32_t curent_raw_inst, uint32_t opcode, Core* cpu){
        //op[0] = get_bit_section(curent_raw_inst,10,5);
        //op[1] = get_bit_section(curent_raw_inst,5,5);
        //op[2] = get_bit_section(curent_raw_inst,0,5);
        op[0] = cpu->add_offset(get_bit_section(curent_raw_inst,10,5),0,opcode);// add offset
        op[1] = cpu->add_offset(get_bit_section(curent_raw_inst,5,5),1,opcode);// add offset
        op[2] = cpu->add_offset(get_bit_section(curent_raw_inst,0,5),2,opcode);// add offset
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


/*
put befor to copy paste instead of
    inline __attribute__((always_inline))   


*/

