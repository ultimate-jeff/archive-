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

using namespace std;
using namespace sf;

class Display_manager{
    public:
        int Tfps = 32768;
        int window_size[2] = {800,600};
        int pixel_size = 100;
        int pixel_amount[2];
        vector<vector<RectangleShape>> pixels;
        RenderWindow window;
        Display_manager(){
            this->pixel_amount[0] = this->window_size[0]/this->pixel_size;
            this->pixel_amount[1] = this->window_size[1]/this->pixel_size;
        }
    void init(){
        this->window.create(VideoMode(this->window_size[0],this->window_size[1]),"g20 display");
        this->window.setFramerateLimit(this->Tfps);
    }
    void clock(){
        this->d_event();
        this->window.clear();


        this->window.display();
         
    }
    private:
        void d_event(){
            sf::Event event;
            while (this->window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    this->window.close();
            }
        }
        void display_pixels(vector<vector<int>>color_map){ 
            /*pixel color format 0(type 20b/10b) 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 */
            for(int py = 0 ; py < this->pixel_amount[1]; py++){
                for(int px = 0 ; px < this->pixel_amount[0];px++){
                    int x = px * this->pixel_size;
                    int y = py * this->pixel_size;
                    RectangleShape pixel = this->pixels[px][py];
                    int int_color = color_map[px][py] & b20_mask;
                    if(int_color >= 524288){ // the top bit is on so use 20 bit color

                    }
                    else{ // use 10 bit color

                    }
                }
            }
        }
};

/*
required ports:
device_id
x
y
sx
sy
data (64 ports)
device_flags_in
device_flags_out

*/
namespace dm{
    Display_manager display;
}
class Display{
    uint32_t start_addr;
    uint32_t requierd_regs = 71;
    uint32_t device_id = 1;
    uint32_t ticks_per_clock = 80;
    public:
        Display(){
            dm::display.init();
            this->start_addr = protocall::get_start_addr(this->requierd_regs);
            interface::init_device(this);
            cout << ("created display instance starting at addr " + to_string(this->start_addr) + " and going " + to_string(this->requierd_regs)) << endl;
        }
        void clock(int loops) {
            if (loops % this->ticks_per_clock == 0) {
                print("Display clocked");
                    if(dm::display.window.isOpen()){
                    dm::display.clock();
                }
            }
        }
    private:
        
};