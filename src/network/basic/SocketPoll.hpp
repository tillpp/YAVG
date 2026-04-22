#pragma once
#include "SocketBase.hpp"
#include <vector>
#include <iostream>
#include <chrono>
#include <map>

#if __linux__
#define POLLFD pollfd
#elif WIN32
#define POLLFD WSAPOLLFD
#endif


//TODO: maybe use sparseSet here.
class SocketPoll
{   
    //assert: no doublicate .fd
    std::vector<POLLFD> connections;
    //assert: index always valid
    std::map<int,int>   handle2Index;
    
public:

    /// @brief 
    /// @param ms maximum time to wait 
    bool wait(size_t ms = 100);

    void add   (SocketBase& socket,bool write  = true,bool read = true);
    void remove(SocketBase& socket);
    void remove(int socket);

    bool isWriteable(SocketBase& socket);
    bool isReadable (SocketBase& socket);
};
