
#include "includes.h"


int main(){
    cout << "started reader " << endl;

    Shm mem;

    if (mem.join()){
        cout << "added device "<< endl;
    }
    else{
        cout << "device faled to add" << endl;
    }

    if(!mem.data->device_count.lock){
        mem.data->device_count.count++;
    }

    return 0;
}