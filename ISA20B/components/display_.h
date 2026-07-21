#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <cstdint>
#include <string>
#pragma once
#include "tools.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <cmath>

using namespace std;
using namespace sf;

inline uint32_t scale_bit(uint32_t value ,uint32_t value_bit_size=10,uint32_t bit_size=8){
    return value*((2^bit_size)/((2^value_bit_size)-1));
}
struct alignas(16) Display_stats{
    int Tfps = 32768;
    int window_size[2] = {800,600};
    int pixel_size = 10;
    int pixel_amount[2];
    int resolution = 4800;
};

class Display_manager{
    public:
    int Tfps = 32768;
    int window_size[2] = {800,600};
    int pixel_size = 10;
    int pixel_amount[2];
    int resolution = 4800;
    // (x*W)+y
    RenderWindow window;
    Display_manager(){
        this->pixel_amount[0] = this->window_size[0]/this->pixel_size;
        this->pixel_amount[1] = this->window_size[1]/this->pixel_size;
    }
    void init(){
        this->window.create(VideoMode(this->window_size[0],this->window_size[1]),"g20 display");
        this->window.setFramerateLimit(this->Tfps);
        this->fill(Color::Black);
    }
    inline __attribute__((always_inline)) 
    void clock(){
        this->d_event();

        this->window.display();
         
    }
    inline __attribute__((always_inline)) 
    void display_pixel(uint32_t x,uint32_t y,uint32_t value){
        this->window.draw(this->get_pixel(x,y,value));
    }
    private:
    void d_event(){
        sf::Event event;
        while (this->window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                this->window.close();
        }
    }
    inline __attribute__((always_inline)) 
    void fill(Color color){
        this->window.clear(color);
    }
    inline __attribute__((always_inline))
    uint32_t get_scaled_value(uint32_t value, uint32_t bits){
        uint64_t max_in = (1ULL << bits) - 1;
        return (uint32_t)((uint64_t)value * 0xFFFFFFFFULL / max_in);
    }
    inline __attribute__((always_inline))
    Color get_color(uint32_t value){
        if (value & (1 << 19)){  // type flag set
            uint32_t payload = value & 0x7FFFF; // 19 bits
            return Color(get_scaled_value(payload, 19));
        }
        else{
            uint32_t payload = value & 0x3FF;   // 10 bits
            return Color(get_scaled_value(payload, 10));
        }
    }
    inline __attribute__((always_inline)) 
    RectangleShape get_pixel(uint32_t x,uint32_t y,uint32_t value){
        RectangleShape shape;
        shape.setFillColor(this->get_color(value));
        shape.setPosition(x*this->pixel_size,y*this->pixel_size);
        return shape;
    }
};

/*
required ports:
device_id
x
y
data (64 ports)
device_flags_in
device_flags_out
# smi 2.0
addr
data
flags_in
flags_out


::
1 == display contend of data to disoplay
2 == ....
....


*/
namespace dm{
    Display_manager display;
}
class Display{
    uint32_t start_addr;
    uint32_t requierd_regs = 73;
    uint32_t device_id = 1;
    uint32_t ticks_per_clock = 80;
    public:
    Display(){
        dm::display.init();
        this->start_addr = protocall::get_start_addr(this->requierd_regs);
        interface::init_device(this);
        cout << ("created display instance starting at addr " + to_string(this->start_addr) + " and going " + to_string(this->requierd_regs)) << endl;
        interface::mem->mem[this->start_addr] = this->device_id;
    }
    inline __attribute__((always_inline))
    void clock(int loops) {
        if (loops % this->ticks_per_clock == 0) {
            
        }
    }
};
/*
uint32_t local_x = i % tile_width;
uint32_t local_y = i / tile_width;


uint32_t index = y * width + x;
*/



