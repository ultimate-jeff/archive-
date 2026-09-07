
#include "includes.h"



int main(){

    //shared_memory_object::remove("sm_test");

    //managed_shared_memory segment(create_only,"sm_test",65536);

    //Sdata* data = segment.construct<Sdata>("Sdata")();

    Shm mem;

    mem.create();

    cout << "started" << endl;
    while (mem.data->active){

       std::this_thread::sleep_for(std::chrono::milliseconds(16)); 

    }

    mem.destroy();

    return 0;
}