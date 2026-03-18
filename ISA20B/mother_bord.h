#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
using namespace std;
#pragma once

namespace components{
    #if __has_include("ssd1/ssd.h")
        #include "ssd1/ssd.h"
        #define HAS_SSD1 1.0
    #else
        #define HAS_SSD1 0.0
    #endif
    #if __has_include("ssd2/ssd.h")
        #include "ssd2/ssd.h"
        #define HAS_SSD2 1.0
    #else
        #define HAS_SSD2 0.0
    #endif

    #if __has_include("ram1/ram.h")
        #include "ram1/ram.h"
        #define HAS_RAM1 1.0
    #else 
        #define HAS_RAM1 0.0
    #endif
    #if __has_include("ram2/ram.h")
        #include "ram2/ram.h"
        #define HAS_RAM2 1.0
    #else 
        #define HAS_RAM2 0.0
    #endif

    #if __has_include("usb/usb_controlor.h")
        #include "usb/usb_controlor.h"
        #define HAS_USB 1.0
    #else
        #define HAS_USB 0.0
    #endif
};
//class Memory;

namespace interface{
    Memory *mem;
    vector<function<void(int)>> devices;

    template<typename T>
    void init_device(double has_component){
        if(has_component >= 1){
            T* device = new T();
            devices.push_back([device](int val) {
                device->clock(val);
            });
        }
    }
    void initialize_devices(){
        init_device<components::ram>(HAS_RAM1);
        init_device<components::ram>(HAS_RAM2);
        init_device<components::ssd>(HAS_SSD1);
        init_device<components::ssd>(HAS_SSD2);
    }

    void init(Memory *port_mem){
        print("initilazing devices");
        mem = port_mem;
        initialize_devices();
    }
    void write_port(uint32_t addr,uint32_t value){
        mem->set_addr(addr,value);
    }
    uint32_t read_port(uint32_t addr){
            cout << "read from port " << addr << endl;
            return mem->get_addr(addr);
    }
    void clock(int loops){
        for(int i = 0 ; i < devices.size() ; i++){
            devices[i](loops);
        }
    }
};

/*

--
p1-p8
p8-p16
--
r1:b
r2:e
end_mm











*/