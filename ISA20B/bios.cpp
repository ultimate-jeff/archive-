#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
using namespace nlohmann;
using namespace std;
#include "cpu/main.h" // main_comp , link
#include "components/tools.h"
//#include "mother_bord.h"


json load_config(string path){
    ifstream file(path);
    json data;
    if(!file.is_open()){
        goto fall_back;
    }
    try{
        data = json::parse(file);
    }
    catch(json::parse_error& e){
        goto fall_back;
    }
    fall_back:
    cout << "bios un able to find bios config going to hard bios settings" << endl;

    file.close();
    return data;
}
vector<uint32_t> readTextUints(const string& path) {
    ifstream file(path);
    vector<std::uint32_t> values;
    if (!file.is_open()) {
        file.close();
        return values; // Or handle error
    }
    std::uint32_t val;
    while (file >> val) {
        values.push_back(val);
    }
    file.close();
    return values;
}

vector<uint32_t> load_inst_to_mem(string path="bios_conf.txt",int main_core_id=0){
    vector<uint32_t> inst;
    inst = readTextUints(path);
    for(int i = 0; i < inst.size() ; i++){
        // i need to inject reg print commands heare 
        if(inst[i] > b20_mask){
            print("Special command : " + to_string(inst[i]) + " this inst will not be added to cpu memory");
        }
        else{
            print("loading reg " + to_string(i+1) + " with " + to_string(inst[i])); // having errors with inst loading 
            cores[main_core_id].mem.set_addr(i,inst[i]);
        }
    }
    cout_print_que();
    return inst;
}

class ram;
class ssd;
class usb;
class Display;
#if __has_include("components/ssd.h")
    #include "components/ssd.h"
    #define HAS_SSD1 1
#else
    #define HAS_SSD1 0
#endif
#if __has_include("components/ssd.h")
    #include "components/ssd.h"
    #define HAS_SSD2 1
#else
    #define HAS_SSD2 0
#endif

#if __has_include("components/ram.h")
    #include "components/ram.h"
    #define HAS_RAM1 1
#else 
    #define HAS_RAM1 0
#endif
#if __has_include("components/ram.h")
    #include "components/ram.h"
    #define HAS_RAM2 1
#else 
    #define HAS_RAM2 0
#endif

#if __has_include("components/usb_controlor.h")
    #include "components/usb_controlor.h"
    #define HAS_USB 1
#else
    #define HAS_USB 0
#endif
#if __has_include("components/display.h")
    #include "components/display.h"
    #define HAS_DISPLAY 1
#else
    #define HAS_DISPLAY 0
#endif

void main_loop(Timer timer,int prints_per_tick = 8){
    cout << "starting program" << endl;
    timer.start_time();
    int loops = -1;
    int active_cores = 1;
    while(active_cores >= 1){
        loops++;
        print("on loop " + to_string(loops));
        if (loops % prints_per_tick == 0){
            cout_print_que();
        }
        active_cores = CLOCK(active_cores);
        interface::clock(loops);
    }
    cout_print_que();
    cout << "program ended" << endl;
    double t = timer.get_time();
    cout << "program took " << t << " milliseconds " << endl;
    int cps = loops/(t/1000);
    cout << "the cps was " << cps << endl;
}


void create_device_instances(){
    
}
int main(){

    json conf = load_config("config/bios_config.json");

    uint32_t port_core_id = conf["port_core_id"];

    Timer timer;
    timer.start_time();
    vector<uint32_t> inst = load_inst_to_mem(conf["boot_path"],conf["main_core_id"]);

    interface::init(&cores[port_core_id].mem);
    protocall::init();
    // create_device_instances
    #if HAS_RAM1
        ram ram1;
    #endif
    #if HAS_RAM2
        ram ram2;
    #endif
    #if HAS_SSD1
        ssd ssd1;
    #endif
    #if HAS_SSD2 
        ssd ssd2;
    #endif
    #if HAS_USB
        usb usb1;
    #endif
    #if HAS_DISPLAY
        Display display1;
    #endif
    // -----------

    cout << "loading data to memory took " << timer.get_time() << " milliseconds" << endl;
    
    // main loop
    main_loop(timer,conf["prints_per_tick"]);
    return 0;
}
/*
compile command:   cd "ISA20B"; g++ bios.cpp -o bios -lsfml-graphics -lsfml-window -lsfml-system

or : 

    cd "c:\Users\matth\.c\proj1\ISA20B\" ; if ($?) { g++ bios.cpp -o bios -lsfml-graphics -lsfml-window -lsfml-system } ; if ($?) { .\bios }

    cd "c:\Users\matth\.c\proj1\ISA20B\" ; if ($?) { g++ -O3 bios.cpp -o bios -lsfml-graphics -lsfml-window -lsfml-system } ; if ($?) { .\bios }    



    cd "c:\Users\matth\.c\proj1\ISA20B\" ; if ($?) { g++ -O3 -march=native -flto -fomit-frame-pointer -funroll-loops bios.cpp -o bios -lsfml-graphics -lsfml-window -lsfml-system } ; if ($?) { .\bios }
*/
