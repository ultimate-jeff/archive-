import re

class G20_C_Compiler:
    def __init__(self):
        # Flags and Opcode mapping from G20 ISA
        self.F_TRUE = 1
        self.F_ZERO = 2
        self.opcode_names = {
            "hult": 0, "stall": 1, "lr": 2, "add": 7, "sub": 8,
            "move": 13, "cmp": 14, "jmp": 15, "sdi": 20, "ld_ptr": 26
        }
        self.reset()

    def reset(self):
        self.variables = {}  
        self.next_reg = 14   # Reserve R0-R13 for system functions
        self.labels = {}
        self.raw_asm = []
        self.label_counter = 0

    def get_reg(self, name):
        if name.isdigit(): return name
        if name not in self.variables:
            self.variables[name] = self.next_reg
            self.next_reg += 1
        return self.variables[name]

    def comp(self, text):
        """Compiles C-style text to G20 Assembly mnemonics."""
        self.reset()
        
        # 1. Safety Header: Jump to the start of the code to avoid data overwrite
        self.raw_asm.append("jmp START_CODE")
        
        lines = [l.strip() for l in text.splitlines() if l.strip() and not l.startswith("//")]
        
        for line in lines:
            # Handle labels (e.g., LOOP:)
            if line.endswith(":"):
                self.labels[line[:-1]] = len(self.raw_asm)
                continue
            
            # START_CODE is a special label for the first instruction after the jump
            if not any("START_CODE" in l for l in self.labels):
                self.labels["START_CODE"] = len(self.raw_asm)

            # Pointer Dereference: val = *ptr
            if "*" in line and "=" in line:
                target, source = [p.strip() for p in line.split("=")]
                t_reg = self.get_reg(target)
                p_reg = self.get_reg(source.replace("*", ""))
                self.raw_asm.append(f"ld_ptr {t_reg} {p_reg}")

            # Standard Math and Assignments
            elif "=" in line:
                target, expr = [p.strip() for p in line.split("=")]
                t_reg = self.get_reg(target)
                parts = expr.split()
                
                if len(parts) == 1: # lr reg data
                    if parts[0].isdigit(): self.raw_asm.append(f"lr {t_reg} {parts[0]}")
                    else: self.raw_asm.append(f"move {t_reg} {self.get_reg(parts[0])}")
                elif len(parts) == 3: # add or sdi
                    left, op, right = parts
                    l_reg = self.get_reg(left)
                    if op == "+": self.raw_asm.append(f"add {l_reg} {self.get_reg(right)} {t_reg}")
                    elif op == "-":
                        if right.isdigit(): self.raw_asm.append(f"sdi {l_reg} {right}")
                        else: self.raw_asm.append(f"sub {l_reg} {self.get_reg(right)} {t_reg}")

            # Control Flow: jnz (Jump Not Zero) using G20 cmp logic
            elif "jnz" in line:
                _, var, dest = line.split()
                reg = self.get_reg(var)
                # cmp reg, flags(3), invert_mask(2) checks for NOT ZERO
                self.raw_asm.append(f"cmp {reg} {self.F_TRUE | self.F_ZERO} {self.F_ZERO}")
                self.raw_asm.append(f"jmp {dest}")

        # Label Resolution
        final_output = []
        for inst in self.raw_asm:
            parts = inst.split()
            resolved = [str(self.labels[p]) if p in self.labels else p for p in parts]
            final_output.append(" ".join(resolved))

        final_output.append("!!") # Stop flag for your compiler
        return "\n".join(final_output)

# --- Benchmark 2: Array Summing (Pointer Test) ---
c_code = """
ptr = 100
total = 0
count = 5
LOOP:
val = *ptr
total = total + val
ptr = ptr + 1
count = count - 1
jnz count LOOP
"""

compiler = G20_C_Compiler()
print(compiler.comp(c_code))