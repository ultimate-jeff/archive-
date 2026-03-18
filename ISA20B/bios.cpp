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
#include "mother_bord.h"

json bios_conf;


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

vector<uint32_t> load_inst_to_mem(string path="bios_conf.txt"){
    vector<uint32_t> inst;
    inst = readTextUints(path);
    for(int i = 0; i < inst.size() ; i++){
        // i need to inject reg print commands heare 
        if(inst[i] > b20_mask){
            print("Special command : " + to_string(inst[i]) + " this inst will not be added to cpu memory");
        }
        else{
            print("loading reg " + to_string(i+1) + " with " + to_string(inst[i])); // having errors with inst loading 
            cores[0].mem.set_addr(i,inst[i]);
        }
    }
    cout_print_que();
    return inst;
}

int main(){
    Timer timer;
    timer.start_time();
    uint32_t port_core_id = 0;
    vector<uint32_t> inst = load_inst_to_mem("boot.txt");
    interface::init(&cores[port_core_id].mem);
    
    cout << "loading data to memory took " << timer.get_time() << " milliseconds" << endl;
    
    // main loop
    cout << "starting program" << endl;
    timer.start_time();

    int loops = 0;
    int active_cores = 1;
    while(active_cores >= 1){
        loops++;
        print("on loop " + to_string(loops));
        if (loops % 2 == 0){
            cout_print_que();
        }
        active_cores = CLOCK(active_cores);
        interface::clock(loops);
    }
    cout << "program ended" << endl;
    cout << "program took " << timer.get_time() << " milliseconds " << endl;
    return 0;
}
/*

load prosesor with initilazation code

initialize devices

start the prosesor

*/
