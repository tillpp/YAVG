#include "SocketPoll.hpp"
#include <thread>
#include <assert.h>

void SocketPoll::add(SocketBase& socket,bool write,bool read){
    auto handle = socket.getHandle();
    POLLFD* p = nullptr;
    if(handle2Index.find(handle) != handle2Index.end()){
        // exists
        int index = handle2Index[handle]; 
        p = &connections.at(index);
    }else{
        // doesn't exists
        POLLFD con;
        con.fd = handle;
        con.events  = 0;
        con.revents = 0;
        handle2Index[handle] = connections.size();

        int index = connections.size();
        connections.push_back(con);
        p = &connections.at(index);
    }
    if(write)
        p->events |= POLLWRNORM;
    if(read)
        p->events |= POLLRDNORM;
    
}
void SocketPoll::remove(SocketBase& socket){
    int handle = socket.getHandle();
    remove(handle);
}
void SocketPoll::remove(int handle){
    if(handle2Index.find(handle)!=handle2Index.end())
        return;
    // swap remove
    int index = handle2Index[handle];
    std::swap(connections[index],connections[connections.size()-1]);
    connections.pop_back();
    int otherHandle = connections[index].fd;
    handle2Index[otherHandle] = index;
    handle2Index.erase(handle);
}

bool SocketPoll::wait(size_t ms){
    //reset everything
    for (auto &&con : connections){
        con.revents = 0;
    }
    
#ifdef __linux__
    if(!connections.size()){
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return false;
    }
    int v = poll(connections.data(),connections.size(),ms);
#elif WIN32
    if(!connections.size()){
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return false;
    }
    //WSAPoll requires at least one element in fds[first arg]
    int v = WSAPoll(connections.data(),connections.size(),ms);
#endif
    //remove all invalid sockets:
    
    for (int i = 0; i < connections.size();){
        auto& con = connections[i];
        if(con.revents & (POLLERR|POLLHUP|POLLNVAL)){
            remove(con.fd);
        }
        else {
            i++;
        }
    }
    
    if(v<0){
        perror("poll");
    }
    return v!=0;
}
bool SocketPoll::isWriteable(SocketBase& socket){
    int handle = socket.getHandle();
    if(handle2Index.find(handle)==handle2Index.end())
        return false;
    int index = handle2Index[handle];
    return connections[index].revents & POLLWRNORM;
}
bool SocketPoll::isReadable (SocketBase& socket){
    int handle = socket.getHandle();
    if(handle2Index.find(handle)==handle2Index.end())
        return false;
    int index = handle2Index[handle];
    return connections[index].revents & POLLRDNORM;
}
