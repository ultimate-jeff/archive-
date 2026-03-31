import sys


class Loger:
    def __init__(self):
        self.print_que = ""
        self._errors = []
        self._logs = []
    def log(self,msg):
        self._logs.append(msg+"\n")
    def error(self,msg):
        self._errors.append(msg+"\n")
    def print_errors(self):
        print("--------errors--------")
        for msg in self._errors:
            print(msg)
    def print_log(self):
        print("-------- logs --------")
        for msg in self._logs:
            print(msg)

loger = Loger()
reg_bit_size = 20
reg_size = 2**reg_bit_size
opcode_size = 5
peramiter_size = 31
op_map = [
    [15],# 0
    [15],# 1
    [5,10],# 2
    [5,10],# 3
    [5,10],# 4
    [5,5],# 5
    [5,5],# 6
    [5,5,5],# 7
    [5,5,5],# 8
    [5,5,5],# 9
    [5,5,5],# 10
    [5,5,5],# 11
    [5,5,5],# 12
    [5,5],# 13
    [5,5,5],# 14
    [15],# 15
    [5,10],# 16
    [15],# 17
    [5,5,5],# 18
    [5,10],# 19
    [5,10],# 20
    [5,5,5],# 21
    [5,5,5],# 22
    [5,5,5],# 23
    [15],# 24
    [15],# 25
    [5,5,5],# 26
    [5,5,5],# 27
    [5,10],# 28
    [5,5,5],# 29
    [5,5,5],# 30
    [5,5,5]# 31
]
opcode_map = {
    "hult":0,# none
    "stall":1,#none
    "lr":2,# reg data
    "push":3,# reg t_reg
    "pull":4,#reg t_reg
    "push_ptr":5,#reg_ptr,t_reg_ptr
    "pull_ptr":6,#reg_ptr,t_reg_ptr
    "add":7,# reg_a,reg_b,reg_c
    "sub":8,# reg_a,reg_b,reg_c
    "and":9,# reg_a,reg_b,reg_c
    "nand":10,# reg_a,reg_b,reg_c
    "or":11,# reg_a,reg_b,reg_c
    "xor":12,# reg_a,reg_b,reg_c
    "move":13,#reg_ptr,t_reg_ptr
    "cmp":14,#reg ,flags,invert_mask
    "jmp":15,#address
    "jmp_ptr":16,#address_ptr
    "null":17,#t_reg
    "intrp":18,#t_reg,bottom_reg,top_reg
    "adi":19,# reg,data
    "sdi":20,# rag,data
    "shift_u":21,#reg_a,reg_b,reg_c
    "shift_d":22,#reg_a,reg_b,reg_c
    "cstate":23,#core_id, value > 0 == start core & value = 0 == stop core
    "call":24,# address
    "ret":25,#none
    "ld_ptr":26,#t_reg,reg
    "ld_off_ptr":27,#offset_reg,ptr_reg,r/w/ff
    "ld_off":28,#offset_reg,data
    "soffmb":29,#offrf,core_id
    "push_c":30,#reg,t_reg,core_id
    "pull_c":31,#reg,t_reg,core_id
    # sudo instructions
    "data":0
}

class Util:
    def __init__(self):
        pass
    def remove_coments(self,line,coment_char="#"):
        output_line = ""
        can_go = True
        for char in line:
            if char != coment_char and can_go:
                output_line += char
            else:
                can_go = False
        return output_line
util = Util()
       
# "7 3 3 3" >> 343433434
class Num_comp:
    def __init__(self):
        pass
    def _cont_to_int(self,l_text,line_num):
        l_int = []
        for text in l_text:
            try:
                l_int.append(int(text))
            except ValueError:
                loger.error(f"syntax error : there was {text} when was expecting type(int) on line {line_num}")
                l_int.append(0)
        return l_int
    def _compile_inst(self,text,split_text,line_num):
        try:
            opcode = int(split_text[0])
            instruction = opcode
            layout = op_map[opcode]
            total_param_bits = sum(layout)
            instruction <<= total_param_bits
            current_shift = total_param_bits
            perams = split_text[1:]
        except ValueError:
            loger.error(f"syntax error : invalid opcode {split_text[0]}")

        for i in range(len(layout)):
            size = layout[i] 
            try:
                val = perams[i]
            except IndexError:
                val = 0
                loger.error(f"error instruction on line {line_num} >> {opcode_map[opcode]} : peramiters where not filled in (filling peramiters with 0)")

            max_val = (1 << size) - 1
            if val > max_val and opcode < peramiter_size:
                loger.error(f"Error on line {line_num}: Parameter {i} for {opcode_map[opcode]} is {val}, which exceeds its {size}-bit limit (max {max_val}) : setting value to max")
                val = max_val
            current_shift -= size
            instruction |= (val << current_shift)

            if instruction > reg_size:
                loger.error(f"error overflow on line {line_num}: instruction compiled over {reg_bit_size} bit limit")

        return instruction
    
    def comp(self,text:str):
        sltext = text.splitlines()
        instrructions = []
        t_inst = ""
        line_num = -1
        for line in sltext: # loops over every line
            line_num += 1
            split_text = line.split() # gets split text
            try:
                if len(split_text) <= 1:# manual load
                    instrructions.append(int(split_text[0]))
                else:
                    instrructions.append(int(self._compile_inst(line,self._cont_to_int(split_text,line_num),line_num)))
            except ValueError:
                pass
            except Exception as e:
                print(e)
            else:
                pass
        return instrructions

class keyWord_handeler:
    def __init__(self):
        self.keywords = {
            "!!":self._togle_running,
            ">>":self._shift_program
        }
        self.keyword_type = {
            "!!":"char",
            ">>":"line"
        }
    def remove_keywords(self,line,keyword):
        new_line = ""
        keyword_type = self.keyword_type.get(keyword,None)
        if keyword_type == "char":
            new_line = line.replace(keyword,"")
        elif keyword_type == "line":
            new_line = "0\n"
        else:
            new_line = line
        return new_line
    def find_keyword(self,line,line_num):
        for key in self.keywords:
            if key in line:
                return key
        return None
    def run_keyword(self,key,line,line_num,comp):
        func = self.keywords.get(key,None)
        if func != None:
            line = func(line,line_num,comp)
        return line

    def _togle_running(self,line,line_num,comp):
        comp.running = not comp.running
        loger.log(f"toggled compalation to {comp.running} on line {line_num} (replacing with opcode 0 / hult)")
        return "0\n"
    def _shift_program(self,line,line_num,comp):
        try:
            shift_amount = line.split()[1]
            shift_amount = int(shift_amount)
        except ValueError:
            loger.error(f"syntax error : invalid shift amount {shift_amount} on line {line_num} (filling shift amount with 0)")
            shift_amount = 0
        except IndexError:
            loger.error(f"syntax error : no shift amount provided on line {line_num} (filling shift amount with 0)")
            shift_amount = 0
        comp.total_shift = shift_amount-1
        return line

class Char_comp:
    def __init__(self):
        self.init()
        self.kwark = keyWord_handeler()
        
    def init(self):
        self.running = True
        self.total_shift = 0
    def _apply_shift(self,line,line_num):
        new_line = ""
        if self.total_shift > 0:
            new_line = "0\n" * self.total_shift
            self.total_shift = 0
            loger.log(f"applied program shift of {self.total_shift} on line {line_num}")
            return new_line
        else:
            return new_line

    def _decode_opcode(self,text,line_num,split_line):
            opcode = opcode_map.get(split_line[0],None)
            if opcode == None:
                opcode = 0
                loger.error(f"syntax error : invalid opcode {split_line[0]} on line {line_num} (replacing opcode with opcode {opcode})")
            return opcode
    def _main_comp(self,line,line_num):
        line = util.remove_coments(line)
        split_line = line.split()
        opcode = None
        if line == "":
            loger.log(f"line {line_num} is empty (filling it with 0)")
            split_line = ["0"]
        elif not split_line[0].isdigit():
            opcode = str(self._decode_opcode(line,line_num,split_line))
            split_line[0] = opcode
        else:
            split_line = [split_line[0]]
        return " ".join(split_line)+"\n"
    
    def comp(self,text:str):
        self.init()
        lines = text.splitlines()
        instructions = ""
        for line_num, line in enumerate(lines, start=1):
            if self.running:
                keyword = self.kwark.find_keyword(line,line_num)
                if keyword:
                    line = self.kwark.run_keyword(keyword,line,line_num,self)
                    line = self.kwark.remove_keywords(line,keyword)
                print(f"compiled line {line_num} : {line}")
                instructions += self._main_comp(line,line_num)
                instructions += self._apply_shift(line,line_num)
        return instructions

def ccomp(text):
    global loger
    num_comp = Num_comp()
    char_comp = Char_comp()
    #result = num_comp.comp(text)
    #result = char_comp.comp(text)
    result = num_comp.comp(char_comp.comp(text))
    text_out = ""
    for line in result:
        print(line)
        text_out += str(line)+"\n"
    print()
    try:
        with open("ISA20B/boot.txt","w") as f:
            f.write(text_out)
    except Exception as e:
        print(f"error writing to file : {e}")
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
