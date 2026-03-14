#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <cstdint>
#include <string>
using namespace std;

class Memory; // the memory class of the cpu 
namespace dmir{
    // actual ram class
    class ram{
        int cpt = 10;
        int addr_size = 15;
        uint32_t size_mask = 32768;
        Memory mem;
        uint32_t port_map_num = 0;
    public:

        ram(Memory mem){
            this->mem = mem;
            cout << "created a ram " << endl;
        }
    private:
        uint32_t mask(uint32_t value,uint32_t mask){
            return value & mask;
        }
    };
}