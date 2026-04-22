#pragma once
#include "SocketBase.hpp"
#include "TcpSocket.hpp"

class TcpListener:public SocketBase
{
public:
    bool listen (std::u8string port);
    bool accept (TcpSocket& client,std::string& ipAddr,int& port);
};
