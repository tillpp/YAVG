#pragma once
#include "SocketBase.hpp"


struct IPAddress{
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof addr;

    IPAddress()=default;
    IPAddress(std::u8string ip,int port);
    void toString(std::string& ipAddr,int& port);
};