
#define BOOST_DATE_TIME_NO_LIB


#include <iostream>
#include <cstdint>
#include <vector>
#include <thread>
#include <chrono>
#include <array>
#include <utility>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>


#pragma once

using std::cout;
using std::endl;
using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::vector;
using std::array;
using std::pair;

//using namespace boost::interprocess;
using boost::interprocess::managed_shared_memory;
using boost::interprocess::interprocess_mutex;
using boost::interprocess::shared_memory_object;
using boost::interprocess::create_only;
using boost::interprocess::open_only;

#define SHM_NAME "jeffydave"
#define STRUCT_NAME "Sdata"
#define MAX_REQUESTS 16
#define DEFALT_LOCK_TIMEOUT_MS 150

enum status_map : int8_t {
    not_avalable = -4,
    timed_out = -3,
    lock_error = -2,
    error = -1,
    none = 0,
    complete = 1,
    active = 2,
    pending = 3,
    un_initalized = 4,
    qued = 5
};
enum mode_map : uint8_t{
    None = 0,
    read = 1,
    write = 2,
    transfer = 3
};

struct Request{
    int8_t status=0;
    uint8_t sending_cid=0;
    uint8_t reciving_cid=0;
    uint8_t mode=0; // 0 = null , 1=read , 2=write, 3=transefer
    uint8_t sending_memory_unit=0;
    uint8_t reaciving_memory_unit=0;
    uint32_t addr=0;
    uint32_t data=0;
};
struct Request_slot{
    interprocess_mutex mutex;
    uint8_t lock_owner;
    bool in_use = false;
    uint64_t lock_timestamp;
    Request request;
};

struct Request_ptr{
    uint8_t index;
    int8_t status;
};
struct Slot_ptr{
    Request_slot* slot;
    int8_t status;
};

struct Sdata{
    interprocess_mutex mutex;
    bool locked;
    bool active = true;
    array<Request_slot,MAX_REQUESTS> requests;
};


uint64_t current_timestamp_ms(){ // gets timestamp
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

class Shm{
private:
    
public:
    managed_shared_memory segment;
    Sdata* data = nullptr;
    Shm() {
    }

    void create(){
        shared_memory_object::remove(SHM_NAME);
        segment = managed_shared_memory(create_only,SHM_NAME,65536);
        data = segment.construct<Sdata>(STRUCT_NAME)();
    }
    bool join(){
        segment = managed_shared_memory(open_only,SHM_NAME);
        auto res = segment.find<Sdata>(STRUCT_NAME);
        data = res.first;
        return data != nullptr;
    }
    bool leve(){
        data = nullptr;
        segment = managed_shared_memory();
        return true;
    }
    void destroy(){
        if(data){
            segment.destroy<Sdata>(STRUCT_NAME);
            data = nullptr;
        }
        segment = managed_shared_memory();
        shared_memory_object::remove(SHM_NAME);
    }

    bool _lock_tmeout(interprocess_mutex& m, unsigned timeout_ms = 100){
        auto deadline = boost::posix_time::microsec_clock::universal_time()
            + boost::posix_time::milliseconds(timeout_ms);
        return m.timed_lock(deadline);
    }
    bool _lock_immediately(interprocess_mutex& m){
        return m.try_lock();
    }
    // mutex.unlock() will un lock the mutex
    int8_t unlock(Request_ptr ptr, bool forced = false, uint8_t cid = 0){
        Request_slot& slot = data->requests[ptr.index];
        if(slot.lock_owner != cid && !forced){ // checks if its you own it then check if forced
            return status_map::lock_error;
        }
        slot.mutex.unlock();
        return status_map::complete;
    }
    int8_t unlock(Request_slot& slot, bool forced = false, uint8_t cid = 0){
        if(slot.lock_owner != cid && !forced){ // checks if its you own it then check if forced
            return status_map::lock_error;
        }
        slot.mutex.unlock();
        return status_map::complete;
    }
    int8_t lock(Request_ptr ptr, bool forced=false, uint8_t cid=0,uint64_t timeout_ms=3){
        Request_slot& slot = data->requests[ptr.index];
        if(slot.lock_owner != cid && !forced){
            return status_map::lock_error;
        }
        if(!_lock_tmeout(slot.mutex, timeout_ms)){ // check if it obtaned the lock with in the time frame
            return status_map::timed_out; 
        }
        slot.lock_timestamp = current_timestamp_ms();
        return status_map::complete;
    }
    int8_t lock_imm(Request_ptr ptr, bool forced=false, uint8_t cid=0){
        Request_slot& slot = data->requests[ptr.index];
        if(slot.lock_owner != cid && !forced){
            return status_map::lock_error;
        }
        if(!_lock_immediately(slot.mutex)){ // check if it obtaned the lock with in the time frame
            return status_map::timed_out; 
        }
        slot.lock_timestamp = current_timestamp_ms();
        return status_map::complete;
    }
    void refresh_lock(Request_slot& slot){
        slot.lock_timestamp = current_timestamp_ms();
    }
    void refresh_lock(Request_ptr ptr){
        Request_slot& slot = data->requests[ptr.index];
        slot.lock_timestamp = current_timestamp_ms();
    }

    bool is_stale(Request_ptr ptr,uint64_t max_age_ms = 200){
        Request_slot& slot = data->requests[ptr.index];
        return (current_timestamp_ms() - slot.lock_timestamp) > max_age_ms;
    }
    bool is_stale(Request_slot& slot,uint64_t max_age_ms = 200){
        return (current_timestamp_ms() - slot.lock_timestamp) > max_age_ms;
    }
    bool force_unlock_if_stale(Request_ptr ptr,uint64_t max_age_ms = 200,uint8_t cid=0){
        if(is_stale(ptr,max_age_ms)){
            unlock(ptr,true,cid);
            return true;
        }
        return false;
    }
    int8_t acquire(Request_slot& slot, uint8_t cid, unsigned timeout_ms = 100){
        if(!_lock_tmeout(slot.mutex, timeout_ms)){
            return static_cast<int8_t>(status_map::timed_out);
        }
        slot.lock_timestamp = current_timestamp_ms();
        slot.lock_owner = cid;
        return status_map::complete;
    }

    void free_stale_requests(uint64_t max_age_ms = 200){
        for(int i = 0 ; i < MAX_REQUESTS ; i++){
            Request_slot& slot = data->requests[i];
            if(is_stale(slot,max_age_ms)){
                unlock(slot,true);
            }
        }
    }
    bool force_unlock_if_stale(Request_ptr ptr, uint64_t max_age_ms = 200){
        Request_slot& slot = data->requests[ptr.index];
        if(!is_stale(ptr, max_age_ms)){
            return false;
        }
        slot.mutex.~interprocess_mutex();
        new (&slot.mutex) interprocess_mutex();
        slot.in_use = false;
        slot.lock_owner = 0;
        slot.lock_timestamp = 0;
        return true;
    }
    Slot_ptr get_slot_ptr(Request_ptr ptr,uint8_t cid,uint64_t time_out = 100){
        Slot_ptr slot_ptr;
        slot_ptr.status = status_map::complete;
        slot_ptr.slot = &data->requests[ptr.index];
        int8_t lock_result = acquire(*slot_ptr.slot,cid,time_out);
        if(lock_result < 0){
            slot_ptr.status = lock_result;
        }
        if(slot_ptr.slot->lock_owner != cid){
            slot_ptr.status = status_map::lock_error;
        }
        return slot_ptr;
    }

    Request_ptr find_open_request(){
        Request_ptr ptr;
        boost::interprocess::scoped_lock<interprocess_mutex> lock(data->mutex, boost::interprocess::defer_lock);
        if(!lock.timed_lock(boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(DEFALT_LOCK_TIMEOUT_MS))){
            ptr.status = status_map::timed_out;
            return ptr;
        }
        for(int i = 0 ; i < MAX_REQUESTS ; i++){
            Request_slot& slot = data->requests[i];
            if(!slot.in_use){
                slot.in_use = true;
                ptr.index = i;
                ptr.status = status_map::complete;
                return ptr;
            }
        }
        ptr.status = status_map::not_avalable;
        return ptr;
    }

    int8_t create_request(Request request,uint8_t cid,bool start_locked = true){
        Request_ptr ptr = find_open_request();
        int8_t exit_code = status_map::complete;
        if(ptr.status < 0){
            return ptr.status;
        } // this finds an open request and returns the error if i couldedt find one

        Slot_ptr slot_ptr = get_slot_ptr(ptr,cid,DEFALT_LOCK_TIMEOUT_MS);
        if(slot_ptr.status >= 0){ // no errors
            slot_ptr.slot->request = request;
            slot_ptr.slot->in_use = true;
            slot_ptr.status = status_map::complete;

            // i was working on this func to make it clame a Request
        }
        slot_ptr.slot->mutex.unlock();
        return slot_ptr.status;
    }
    int8_t free_request(Request_ptr ptr,uint8_t cid){
        Slot_ptr slot_ptr = get_slot_ptr(ptr,cid,DEFALT_LOCK_TIMEOUT_MS);
        Request_slot& slot = *slot_ptr.slot;
        if(slot_ptr.status >= 0){
            slot.in_use = false;
            slot.lock_owner = 0;
        }
        slot.mutex.unlock();
        return slot_ptr.status;
    }

    int8_t edit_request(Request_ptr ptr,Request new_request,uint8_t cid){
        Slot_ptr slot_ptr = get_slot_ptr(ptr,cid,DEFALT_LOCK_TIMEOUT_MS);
        if(slot_ptr.status >= 0){
            slot_ptr.slot->request = new_request;
        }
        slot_ptr.slot->mutex.unlock();
        return slot_ptr.status;
    }


};


