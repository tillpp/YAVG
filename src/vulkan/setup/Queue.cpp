#include "Queue.hpp"

Queue::Queue():vk::raii::Queue(nullptr){

}

void Queue::create(DeviceSettings& deviceSettings){
    deviceSettings.queues.push_back(this);
}
