/////////////////////
     geco 20 isa
/////////////////////
by Batthew R and William L

info:
    this is a 20 bit custom cpu emulator that is baced on the intel 8086 prosesor and the youtuber's Mattbatwings minecraft cpu
    this emulator also comes with a compiler for it 
    anyone can use this emulator as they please and i encurage you to build and learn from this emulator 

specs:
- has 8 cores
- has 8 offset memory banks
- each core has 4096 general pourpos regesters
- each core has its own callstack with a resesion depth of 255 
- it has a dedicated port core
- all general pourpos regesters are 20 bit

cpu data:
    regester_format:
        - data_reg:(top 5 un alocated),(next 5 down is flags),(bottom 10 is data) , example: 00000 10000 1111111111
        - instruction_reg:(top 5 is the opcode),(bottom 15 is peramiters) , example: 00000 000000000000000
    offsets:
        - there are 8 offset memory banks and each has 32 offsets and each core gets 4 so if i do lr 3 10 or load regester 3 with 10 offset 0 for that core is added to the first peramiter so it would be peramiter+offset == actual but some peramiters can block offsets like this lr instruction bc the first peramiter has offsets but the second does not
        - each core has its active offser memory bank abd it can switch which one it is using so if offset0 on offset memory bank 0 is 3 and i switch banks that could be a difrent number so essentaly hardwhere context switching 
        - each core can mod any offset in its curent offset memory bank
        - examples :
            - lr 3 10   in this example the lr instruction takes the offset on the first peramiter bc the addr is 5 bit but full memory is 12 bit so the offsets allow you to acses aryas of memory that could not normaly be done 
            - add r1 r2 r3   in this example add uses 3 offsets (the 4th is un used) 
    instruction_set:
        offset0 to peramiter1 and offset 1 to peramiter2 .....
         opcode  peramiters(use_offset)
          5bit    5bit    5bit    5bit
        |  hult |       |       |       |
        | stall |       |       |       |
        |lr     |reg    |      data     |
        |push   |reg    |      t_reg    |
        |pull   |reg    |      t_reg    |
        |pushPtr|ptr_Reg|TptrReg|       |
        |pullPtr|ptr_Reg|TptrReg|       |
        |add    |regA   |regB   |regD   |
        |sub    |regA   |regB   |regD   |
        |and    |regA   |regB   |regD   |
        |nand   |regA   |regB   |regD   |
        |or     |regA   |regB   |regD   |
        |xor    |regA   |regB   |regD   |
        |move   |reg_ptr|TregPtr|       |
        |cmp    |reg    |flags  |invertM|
        |jmp    |          addr         |
        |jmp_ptr|addrPtr|       |       |
        |intrp  |core_id|addrPtr|       |
        |adi    |reg    |      data     |
        |sdi    |reg    |      data     |
        |shift_U|regA   |regB   |regD   |
        |shift_D|regA   |regB   |regD   |
        |Cstate |core_id|1 < = s|       |
        |call   |          addr         |
        |ret    |       |       |       |
        |ld_ptr |t_reg  |reg    |       |
        |LOffPtr|offset |ptr_reg|       |
        |SoffMB |offMB  |core_id|       |
        |push_c |reg    |T_reg  |core_id|
        |pull_c |reg    |T_reg  |core_id|


how to use:
- to config the bios setting u can go to the json "bios_config.json" at "ISA20B/config/bios_config.json" and this will be where u tell the bios where the boot file is and other stuff NOTE(the boot is the exact data that will be loaded to the main core befor starting so it cant have any junk / use the compiler)
- after u have the boot program in the boot file and the bios config is set up u can atach devices such as "ISA20B/components/ram.h" and "ISA20B/components/ssd.h" and "ISA20B/components/usb_controlor.h" and these will be included by the bios and if u use the protocall they will be initalized 
- after u have all that u can compile the bios and run it with c++ > 15.2.0 NOTE(there might be some other stuff u need to do befor compiling and running)
