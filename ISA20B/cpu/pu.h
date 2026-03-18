#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#pragma once
#include "global_utils.h"

using namespace std;
struct Flag{
    bool True;
    bool zero;
    bool carry;
    bool overflow;
    bool invert;
}; 

uint32_t comp_flags(Flag f){
    uint32_t result = 0;
    if (f.True) result |= b1_mask;            // bit 0 of flags (bit 10 of reg)
    if (f.zero) result |= (b1_mask << 1);     // bit 1 of flags (bit 11 of reg)
    if (f.carry) result |= (b1_mask << 2);    // bit 2 of flags (bit 12 of reg)
    if (f.overflow) result |= (b1_mask << 3); // bit 3 of flags (bit 13 of reg)
    if (f.invert) result |= (b1_mask << 4);   // bit 4 of flags (bit 14 of reg)
    return result;
}

Flag decomp_flags(uint32_t value){
    Flag f;
    f.True     = (value >> 0) & 1;
    f.zero     = (value >> 1) & 1;
    f.carry    = (value >> 2) & 1;
    f.overflow = (value >> 3) & 1;
    f.invert   = (value >> 4) & 1;
    return f;
}
class ALU{
public:
    ALU(){};
   uint32_t gen_flags(uint32_t raw_result, uint32_t a, uint32_t b, bool is_sub = false){
        Flag f{};
        uint32_t result = mask(raw_result, b10_mask);
        uint32_t sign = sine_mask(b10_mask);   // sign bit (bit 9)
        f.zero = (result == 0);
        f.carry = (raw_result > b10_mask);
        
        bool sign_a = (a & sign);
        bool sign_b = (b & sign);
        bool sign_r = (result & sign);
        
        if (is_sub) {
            f.overflow = (sign_a != sign_b) && (sign_r != sign_a);
        } else {
            f.overflow = (sign_a == sign_b) && (sign_r != sign_a);
        }
        
        f.True = true;
        f.invert = (result & sign) != 0;
        return comp_flags(f);
    }
    pair<uint32_t,uint32_t> ADD(uint32_t a, uint32_t b){
        uint32_t raw_raw = a + b;
        uint32_t raw = mask(raw_raw, b10_mask);
        return {raw, gen_flags(raw_raw,a,b)};
    }
    pair<uint32_t,uint32_t> SUB(uint32_t a, uint32_t b){
        uint32_t raw_raw = a - b;
        uint32_t raw = mask(raw_raw, b10_mask);
        return {raw, gen_flags(raw_raw,a,b,true)};
    }
    pair<uint32_t,uint32_t> AND(uint32_t a, uint32_t b){
        uint32_t raw = mask(a & b, b10_mask);
        return {raw, gen_flags(raw,a,b)};
    }
    pair<uint32_t,uint32_t> NAND(uint32_t a, uint32_t b){
        uint32_t raw = mask(~(a & b), b10_mask);
        return {raw, gen_flags(raw,a,b)};
    }
    pair<uint32_t,uint32_t> OR(uint32_t a, uint32_t b){
        uint32_t raw = mask(a | b, b10_mask);
        return {raw, gen_flags(raw,a,b)};
    }
    pair<uint32_t,uint32_t> XOR(uint32_t a, uint32_t b){
        uint32_t raw = mask(a ^ b, b10_mask);
        return {raw, gen_flags(raw,a,b)};
    }
    pair<uint32_t,uint32_t> NOT(uint32_t a,uint32_t b){ // takes a b for standerd format
        uint32_t raw = mask(~a, b10_mask);
        return {raw, gen_flags(raw,a,0)};
    }
};
class PU{
public:
    ALU alu;
    uint32_t regA = 0;
    uint32_t regB = 0;
    uint32_t loads = 0;
    uint32_t out_reg = 0;
    int curent_inst = 0;
    PU(){};
    uint32_t comp_reg(uint32_t b10bit, uint32_t mid5bit, uint32_t t5bit){
        uint32_t result = 0;
        result |= (t5bit << (alu_data_size + 5)); // Bit 15+
        result |= (mid5bit << alu_data_size);     // Bit 10-14 (FLAGS)
        result |= b10bit;                         // Bit 0-9
        return mask(result, b20_mask);
    }
    pair<uint32_t,uint32_t> decomp_reg(uint32_t reg){
        uint32_t b10bit = get_bit_section(reg, 0, alu_data_size);
        uint32_t mid5bit = get_bit_section(reg, alu_data_size, 5);
        uint32_t t5bit = get_bit_section(reg, alu_data_size + 5, 5);
        return {b10bit, t5bit};
    }

    uint32_t ADD(uint32_t regA, uint32_t regB){
        auto [a, tA] = decomp_reg(regA);
        auto [b, tB] = decomp_reg(regB);
        auto [result, flags] = alu.ADD(a, b);
        print("on op add the flags are " + to_string(flags));
        this->out_reg = comp_reg(result, flags, tA);
        return this->out_reg;
    }
    uint32_t SUB(uint32_t regA, uint32_t regB){
        auto [a, tA] = decomp_reg(regA);
        auto [b, tB] = decomp_reg(regB);
        auto [result, flags] = alu.SUB(a, b);
        print("on op sub the flags are " + to_string(flags));
        this->out_reg = comp_reg(result, flags, tA);
        return this->out_reg;
    }
    uint32_t AND(uint32_t regA, uint32_t regB){
        auto [a, tA] = decomp_reg(regA);
        auto [b, tB] = decomp_reg(regB);
        auto [result, flags] = alu.AND(a, b);
        this->out_reg = comp_reg(result, flags, tA);
        return this->out_reg;
    }
    uint32_t NAND(uint32_t regA, uint32_t regB){
        auto [a, tA] = decomp_reg(regA);
        auto [b, tB] = decomp_reg(regB);
        auto [result, flags] = alu.NAND(a, b);
        this->out_reg = comp_reg(result, flags, tA);
        return this->out_reg;
    }
    uint32_t OR(uint32_t regA, uint32_t regB){
        auto [a, tA] = decomp_reg(regA);
        auto [b, tB] = decomp_reg(regB);
        auto [result, flags] = alu.OR(a, b);
        this->out_reg = comp_reg(result, flags, tA);
        return this->out_reg;
    }
    uint32_t XOR(uint32_t regA, uint32_t regB){
        auto [a, tA] = decomp_reg(regA);
        auto [b, tB] = decomp_reg(regB);
        auto [result, flags] = alu.XOR(a, b);
        this->out_reg = comp_reg(result, flags, tA);
        return this->out_reg;
    }
    uint32_t NOT(uint32_t regA,uint32_t regB){ // takes regB for standerd format
        auto [a, tA] = decomp_reg(regA);
        auto [result, flags] = alu.NOT(a,0);
        this->out_reg = comp_reg(result, flags, tA);
        return this->out_reg;
    }
    uint32_t shift_up(uint32_t regA, uint32_t amount){
        regA = regA << amount;
        this->out_reg = mask(regA, b20_mask);
        return this->out_reg;
    }
    uint32_t shift_down(uint32_t regA, uint32_t amount){
        regA = regA >> amount;
        this->out_reg = mask(regA, b20_mask);
        return this->out_reg;
    }
    
    void load_reg(uint32_t value){
        this->loads++;
        switch (this->loads % 2){
        case 0:this->regB = value;break;  
        case 1:this->regA = value;break;
        }
    }
    uint32_t get_out_reg(){
        return this->out_reg;
    }
    void set_op(string op){
        if (op == "ADD") this->curent_inst = 0;
        else if (op == "SUB") this->curent_inst = 1;
        else if (op == "AND") this->curent_inst = 2;
        else if (op == "NAND") this->curent_inst = 3;
        else if (op == "OR") this->curent_inst = 4;
        else if (op == "XOR") this->curent_inst = 5;
        else if (op == "NOT") this->curent_inst = 6;
        else if (op == "SHL") this->curent_inst = 7;
        else if (op == "SHR") this->curent_inst = 8;
        else {
            cout << "Invalid operation: " << op << "seting opcode to 0 / ADD"<<endl;
            this->curent_inst = 0; 
        }
    }
    void set_op(int opcode){
        if (opcode >= 0 && opcode <= 8){
            this->curent_inst = opcode;
        } else {
            cout << "Invalid opcode: " << opcode << "seting opcode to 0 / ADD"<<endl;
            this->curent_inst = 0; 
        }
    }

    uint32_t exec_op(uint32_t regA, uint32_t regB, uint32_t opcode=-1){
        if (opcode == -1){
            opcode = this->curent_inst;
        }
        switch (opcode) {
            case 7: return ADD(regA, regB);
            case 8: return SUB(regA, regB);
            case 9: return AND(regA, regB);
            case 10: return NAND(regA, regB);
            case 11: return OR(regA, regB);
            case 12: return XOR(regA, regB);
            case 33: return NOT(regA, regB);
            case 21: return shift_up(regA, regB);
            case 22: return shift_down(regA, regB);
            default: throw invalid_argument("Invalid opcode"+to_string(opcode));
        }
    }
};