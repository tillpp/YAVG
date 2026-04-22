#pragma once
#include "SocketBase.hpp"
#include "IPAddress.hpp"

class UdpSocket:public SocketBase
{
    
public:   
    UdpSocket(UdpSocket&)=delete;

    bool bind(std::u8string port);
    // to ephemeral port / a.k.a. dynamic random port
    bool bind();
    bool send(const char*  msg,size_t size,size_t& transmitted,const IPAddress& ip);
    bool recv(char* recvbuf,size_t size,size_t& received,IPAddress& ip);
};