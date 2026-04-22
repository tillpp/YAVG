#pragma once

#ifdef __linux__
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#endif

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#endif

#include <string>

#ifdef __linux__
const int INVALID_SOCKET = -1;
typedef int SOCKET; 
#endif 

class SocketBase
{
    friend class TcpListener;
protected:

    SOCKET handle = INVALID_SOCKET;
    bool receivable  = true;
    bool sendable    = true;
    void setHandle(SOCKET handle);

public:

    SocketBase(/* args */);
    virtual ~SocketBase();

    void close();
    [[deprecated]] bool shutdown(bool shutdown_read,bool shutdown_write);
    bool exist();
    SOCKET getHandle();

};

