#include "UdpSocket.hpp"

#include <iostream>
#include <cstring>
#include <codecvt>
#include <locale>
#include <string>
#include <assert.h>

bool UdpSocket::bind(){
#if defined(__linux__) || defined(WIN32)
    return bind(u8"0");
#else
    #error ephemeral port binding for other OS
#endif
}
bool UdpSocket::bind(std::u8string port){
    close();
#if __linux__
    struct addrinfo *result = NULL, *ptr = NULL, hints;

    memset(&hints,0,sizeof(hints));
    hints.ai_family   = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags    = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    int iResult = getaddrinfo(NULL, (char*)port.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(iResult));
        freeaddrinfo(result);
        return false;
    }
    // Attempt to connect to the first possible address returned by
    // the call to GetAddrInfo
    for (struct addrinfo* rp = result; rp != NULL; rp = rp->ai_next) {
        // Create a SOCKET for the server to listen for client connections
        handle = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (handle == INVALID_SOCKET)
            continue;

        // "Dual-Stack Sockets" FOR BOTH IPv4 and IPv6 socket! Before BIND!
        if(result->ai_family==AF_INET6){
            int no = 0;
            setsockopt(handle,IPPROTO_IPV6,IPV6_V6ONLY,(const char*)&no,sizeof(no));
        }
        //TODO: testing only
        {
            int no = 1;
            setsockopt(handle,SOL_SOCKET,SO_REUSEPORT,(const char*)&no,sizeof(no));
            
        }

        // Setup the TCP listening socket
        if (::bind(handle, rp->ai_addr, (int)rp->ai_addrlen) == 0)
            break;

        //close socket again
        if(::close(handle)<0)
            perror("close");
        handle = INVALID_SOCKET;
    }
    freeaddrinfo(result);           
    if (handle == INVALID_SOCKET) {
        perror("socket(),bind()\n");
        return false;
    }
    return true;
#elif WIN32
    struct addrinfoW *result = NULL, hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring portWide = converter.from_bytes(reinterpret_cast<const char*>(port.c_str()));
    int iResult = ::GetAddrInfoW(NULL, portWide.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("GetAddrInfoW failed: %d\n", iResult);
        FreeAddrInfoW(result);
        return false;
    }
    // Attempt to connect to the first possible address returned by
    // the call to GetAddrInfo
    for (struct addrinfoW* rp = result; rp != NULL; rp = rp->ai_next) {
        // Create a SOCKET for the server to listen for client connections
        handle = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (handle == INVALID_SOCKET)
            continue;

        // "Dual-Stack Sockets" FOR BOTH IPv4 and IPv6 socket! Before BIND!
        if(result->ai_family==AF_INET6){
            DWORD no = 0;
            ::setsockopt(handle,IPPROTO_IPV6,IPV6_V6ONLY,(const char*)&no,sizeof(no));
        }
        
        // Setup the TCP listening socket
        if (::bind(handle, rp->ai_addr, (int)rp->ai_addrlen) == 0)
            break;         

        //close socket again
        if(::closesocket(handle)<0)
            perror("close");
        handle = INVALID_SOCKET;
    }
    FreeAddrInfoW(result);           
    if (handle == INVALID_SOCKET) {       
        printf("socket(),bind(): %ld\n", WSAGetLastError());
        return false;
    }
    return true;
#endif
}

bool UdpSocket::send(const char* msg,size_t size,size_t& transmitted,const IPAddress& ip){
    if(!exist() || !sendable)
        return false;
#ifdef __linux__
    //Send an initial buffer
    
    int iResult = ::sendto(handle, msg, (int) size, 0,(struct sockaddr*)&ip.addr,ip.addr_len);
    if (iResult < 0) {
        perror("send");
        close();
        return false;
    }
    transmitted = iResult;
    return true;
#elif WIN32 //#########################################################
    //Send an initial buffer
    int iResult = ::sendto(handle, msg, (int) size, 0,(struct sockaddr*)&ip.addr,ip.addr_len);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        close();
        return false;
    }
    transmitted = iResult;
    return true;
#endif
}
bool UdpSocket::recv(char* recvbuf,size_t length,size_t& received,IPAddress& ip){
    if(!exist() || !receivable)
        return false;
#ifdef __linux__
    ip.addr_len = sizeof(ip.addr);
    int iResult = ::recvfrom(handle, recvbuf, length, 0, (struct sockaddr *)&ip.addr, &ip.addr_len);
    if (iResult > 0){
        //new length
        received = iResult;           
    }
    else if (iResult == 0){
        //dgrams allow zero byte packets
        received = iResult;           
        return true;
    }
    else{
        perror("recv");
    }
    return iResult > 0;
#elif WIN32
    int iResult = ::recvfrom(handle, recvbuf, length, 0,(struct sockaddr *)&ip.addr, &ip.addr_len);
    if (iResult > 0){
        received = iResult;
    }
    else if (iResult == 0){
        close();
    }
    else
        printf("recv failed: %d\n", WSAGetLastError());
    return iResult > 0;
#endif
}

