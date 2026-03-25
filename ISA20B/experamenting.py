
import string
import sys
import os

reg_bit_size = 20
reg_size = 2**reg_bit_size
opcode_size = 5
peramiter_size = 31
op_map = [
    [15],
    [15],
    [5,10],
    [5,10],
    [5,10],
    [5,5],
    [5,5],
    [5,5,5],
    [5,5,5],
    [5,5,5],
    [5,5,5],
    [5,5,5],
    [5,5,5],
    [5,5],
    [5,5,5],
    [15],
    [5,10],
    [5,5,5],
    [5,5,5],
    [5,10],
    [5,10],
    [5,5,5],
    [5,5,5],
    [5,5,5],
    [15],
    [15],
    [5,5,5],
    [5,5,5],
    [5,10],
    [5,5,5],
    [5,5,5],
    [5,5,5],
    [15]
]
# flags are >> true,zero,carry,overflow,sine
# all regs are 20bit but only the botom 10 bit are data and then the next 5 bit is the flags and then the next 5 are not part of the data struct
# for instructions the top 5 bits are the opcode and then the botom 15 are the peramiters
# the memory size is 2^12 or 4096 regs of shaird memory meaning instructions are in the same memory space as data so be carfle where u put ur data so that u dont exacute data as an instruction 
# there are 4 offsets per core and 0th offset gose to the first peramiter then the 1th offset gose to the second peramiter and then the 2th offset gose to the third peramiter 
# each op has a offset block map which sais which peramiters can use offsets and all reg peramiters use offsets but none of the data peramiters do and none of the core id peramiters use offsets eather 

opcode_map = [
    "hult",# none
    "stall",#none
    "lr",# reg data
    "push",# reg t_reg
    "pull",#reg t_reg
    "push_ptr",#reg_ptr,t_reg_ptr
    "pull_ptr",#reg_ptr,t_reg_ptr
    "add",# reg_a,reg_b,reg_c
    "sub",# reg_a,reg_b,reg_c
    "and",# reg_a,reg_b,reg_c
    "nand",# reg_a,reg_b,reg_c
    "or",# reg_a,reg_b,reg_c
    "xor",# reg_a,reg_b,reg_c
    "move",#reg_ptr,t_reg_ptr
    "cmp",#reg ,flags,invert_mask
    "jmp",#address
    "jmp_ptr",#address_ptr
    "null",#t_reg,bottom_reg,top_reg
    "intrp",#t_reg,bottom_reg,top_reg
    "adi",# reg,data
    "sdi",# rag,data
    "shift_u",#reg_a,reg_b,reg_c
    "shift_d",#reg_a,reg_b,reg_c
    "cstate",#core_id, value > 0 == start core & value = 0 == stop core
    "call",# address
    "ret",#none
    "ld_ptr",#t_reg,reg
    "ld_off_ptr",#offset_reg,ptr_reg,r/w/ff
    "ld_off",#offset_reg,data
    "soffmb",#offrf,core_id
    "push_c",#reg,t_reg,core_id
    "pull_c",#reg,t_reg,core_id
    "int"
]
class Syntax_checker:
    def __init__(self):
        self._errors = []
        self._msgs = []
    def error(self,msg):
        #print(msg)
        self._errors.append(msg)
    def print_errors(self):
        print("!!-- errors --!!")
        for error in self._errors:
            print(error)
    def log(self,msg):
        self._msgs.append(msg)
    def print_log(self):
        print("-- logs --")
        for msg in self._msgs:
            print(msg)
class Util:
    def __init__(self):
        pass
    def remove_coments(self,line):
        output_line = ""
        can_go = True
        for char in line:
            if char != "#" and can_go:
                output_line += char
            elif char == "*" and can_go:
                output_line = "hult 0"
            elif char.isdigit():
                output_line += char
            else:
                can_go = False
        return output_line
loger = Syntax_checker()
util = Util()

class Num_comp:
    def __init__(self):
        pass
    def _encode_instruction(self,opcode, params, op_map, opcode_size=5,line_num=None):
        instruction = opcode
        layout = op_map[opcode]
        total_param_bits = sum(layout)
        instruction <<= total_param_bits
        current_shift = total_param_bits
        for i in range(len(layout)):
            size = layout[i]
            try:
                val = params[i]
            except IndexError:
                val = 0
                if opcode_map[opcode] != "hult" and opcode_map[opcode] != "ret" and not (opcode_map[opcode] > 31):
                    loger.error(f"error instruction on line {line_num} >> {opcode_map[opcode]} : peramiters where not filled in (filling peramiters with 0)")
            max_val = (1 << size) - 1
            if val > max_val and opcode < peramiter_size:
                loger.error(f"Error on line {line_num}: Parameter {i} for {opcode_map[opcode]} is {val}, which exceeds its {size}-bit limit (max {max_val})")

            current_shift -= size
            instruction |= (val << current_shift)
            if instruction > reg_size:
                loger.error(f"error overflow on line {line_num}: instruction compiled over {reg_bit_size} bit limit (croping data to fit)")
                instruction = instruction & (2**reg_bit_size)-1
        return instruction
    def _llnum_comp_line(self,text,line_num):
        text = text.split()
        instruction = []
        for char in text:
            try:
                value = int(char)
                instruction.append(value)
            except ValueError:
                loger.error(f"syntax error on line {line_num}: trying to use invalid chars where intagers should have been")

        final_ints = self._encode_instruction(instruction[0],instruction[1:],op_map,opcode_size,line_num=line_num)
        return final_ints
    def comp(self,text):
        instructios = []
        tspl = text.splitlines()
        for line in tspl:
            line_num = tspl.index(line)+1
            instructios.append(self._llnum_comp_line(line,line_num))
        return instructios

class Char_comp:
    def __init__(self):
        pass
    def _remove_coments(self,line):
        output_line = ""
        can_go = True
        for char in line:
            if char != "#" and can_go:
                output_line += char
            else:
                can_go = False
        return output_line

    def comp(self,text):
        Instruction = ""
        instructions = []
        tspv = text.splitlines()
        for line_num, line in enumerate(tspv, start=1):
            try:
                line = self._remove_coments(line)
                split_line = line.split()
                if line[0] == "!" and line[1] == "!":
                    loger.log(f"ending_compalation on line {line_num}")
                    break
                elif line[0] == "*":
                    split_line.clear()
                    split_line.append("hult")
                opcode = opcode_map.index(split_line[0])# potentioal for invalid opcode
            except IndexError:
                loger.log(f"line {line_num} is empty (skiping copalation of line) ")
            except ValueError:
                if split_line[0].isdigit():
                    pass
                    #print(f"compiled: {line}")
                    #split_line[0] = str(opcode)
                    #new_line = " ".join(split_line)
                    #Instruction += new_line+"\n"
                else:
                    loger.error(f"syntax error on line {line_num} : invalid opcode )")
            except Exception as e:
                loger.error(f"syntax error {e} on line {line_num} (oh crap, you're doomed)")
            else:
                print(f"compiled: {line}")
                split_line[0] = str(opcode)
                new_line = " ".join(split_line)
                Instruction += new_line+"\n"
        return Instruction


class Resoving_comp:
    def comp(self, text):
        Instruction = ""
        instructions = []
        lines = text.splitlines()
        index = 0
        self.line_num = 1
        while index < len(lines) and self.running:
            line = lines[index]
            line = util.remove_coments(line)
            split_line = line.split()
            try:
                opcode = opcode_map.index(split_line[0])
            except Exception as e:
                pass
            else:
                print(f"compiled: {line}")
                split_line[0] = str(opcode)
                new_line = " ".join(split_line)
                Instruction += new_line+"\n"
                
            self.line_num += 1
            index += 1
        return Instruction

    def end_comp(self):
        loger.log(f"ending_compalation on line {self.line_num}")
        self.running = False

    def __init__(self):
        self.running = True
       

def ccomp(text):
    global loger
    resolver = Resoving_comp()
    char_comp = Char_comp()
    num_comp = Num_comp()

    #result = resolver.comp(text)
    result = num_comp.comp(char_comp.comp(text))
    print("----------")
    for line in result:
        print(f"{line}")
    print("----------")
    #print(idk2.comp(text))
    print()
    loger.print_errors()
    loger.print_log()

if __name__ == "__main__":
    # Check if a file path was passed as an argument
    if len(sys.argv) > 1:
        file_to_open = sys.argv[1]
        with open(file_to_open,"r") as f:
            text = f.read()
        ccomp(text)
    else:
        print("no script to compile")
        data = input("enter dir of .txt assembly file :").lower()
        if data == "y" or data == "":
            print("compiling ISA20B/assem.txt \n\n\n")
            with open("ISA20B/assem.txt","r") as f:
                text = f.read()
            ccomp(text)
        else:
            print(f"compiling {data} \n\n\n")
            with open(data,"r") as f:
                text = f.read()
            ccomp(text)
    input("press enter to continue :")
else:
    print("compiling ISA20B/assem.txt \n\n\n")
    with open("ISA20B/assem.txt","r") as f:
        text = f.read()
    ccomp(text)

# python dir PS C:\Users\matth\AppData\Local\Programs\Python\Python313>        
"""
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>

std::vector<std::uint32_t> readTextUints(const std::string& path) {
    std::ifstream file(path);
    std::vector<std::uint32_t> values;
    
    if (!file.is_open()) {
        return values; // Or handle error
    }

    std::uint32_t val;
    while (file >> val) {
        values.push_back(val);
    }

    return values;
}

"""