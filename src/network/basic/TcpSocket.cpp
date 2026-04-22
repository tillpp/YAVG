#include "TcpSocket.hpp"

#include <iostream>
#include <cstring>
#include <codecvt>
#include <locale>
#include <string>
#include <assert.h>


bool TcpSocket::connect(std::u8string ip,std::u8string port){
    close();
#ifdef __linux__
    struct addrinfo *result = NULL,
                    hints;
    memset( &hints,0, sizeof(hints) );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    // Resolve the server address and port
    int iResult = getaddrinfo((const char*)ip.c_str(), (const char*)port.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(iResult));
        freeaddrinfo(result);
        return false;
    }

    // Attempt to connect to the first possible address returned by
    // the call to GetAddrInfo
    for (struct addrinfo* rp = result; rp != NULL; rp = rp->ai_next) {
        // Create a SOCKET for connecting to server
        handle = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (handle == INVALID_SOCKET)
            continue;

        //be able to rebind to 
        #if __linux__
        {
            int on = 1;
	        setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (const void*) &on, (socklen_t) sizeof(on));
        }
        #endif

        // Connect to server.
        if (::connect(handle, rp->ai_addr, (int)rp->ai_addrlen) == 0){
            break;                  /* Success */
        }

        //close socket again
        if(::close(handle)<0)
            perror("close");
        handle = INVALID_SOCKET;
    }
    freeaddrinfo(result);           
    if (handle == INVALID_SOCKET) {
        perror("socket(),connect()\n");
        return false;
    }
    return true;
#elif WIN32 //##############################################################################################################
    struct addrinfoW *result = NULL,
    hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring ipWide   = converter.from_bytes(reinterpret_cast<const char*>(ip.c_str()));
    std::wstring portWide = converter.from_bytes(reinterpret_cast<const char*>(port.c_str()));
    // Resolve the server address and port
    int iResult = GetAddrInfoW(ipWide.c_str(), portWide.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("GetAddrInfoW failed: %d\n", iResult);
        FreeAddrInfoW(result);
        return false;
    }
    
    // Attempt to connect to the first address returned by
    // the call to GetAddrInfoW
    for (struct addrinfoW* rp = result; rp != NULL; rp = rp->ai_next) {
        // Create a SOCKET for connecting to server
        handle = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (handle == INVALID_SOCKET)
            continue;

        // Connect to server.
        if (::connect(handle, rp->ai_addr, (int)rp->ai_addrlen) == 0){
            break;                  /* Success */
        }

        //close socket again
        if(::closesocket(handle)<0)
            perror("close");
        handle = INVALID_SOCKET;
    }
    FreeAddrInfoW(result);           
    if (handle == INVALID_SOCKET) {
        printf("socket(),connect(): %ld\n", WSAGetLastError());
        return false;
    }
    return true;
#endif
}

bool TcpSocket::send(const char* msg,size_t size,size_t& transmitted){
    if(!exist() || !sendable)
        return false;
#ifdef __linux__
    //Send an initial buffer
    
    int iResult = ::send(handle, msg, (int) size, 0);
    if (iResult < 0) {
        perror("send");
        close();
        return false;
    }
    transmitted = iResult;
    return true;
#elif WIN32 //#########################################################
    //Send an initial buffer
    int iResult = ::send(handle, msg, (int) size, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        close();
        return false;
    }
    transmitted = iResult;
    return true;
#endif
}
bool TcpSocket::recv(char* recvbuf,size_t length,size_t& received){
    if(!exist() || !receivable)
        return false;
#ifdef __linux__
    int iResult = ::recv(handle, recvbuf, length, 0);
    if (iResult > 0){
        //new length
        received = iResult;           
    }
    else if (iResult == 0){
        //stream: orderly exit
        close();
    }
    else
        perror("recv");
    return iResult > 0;
#elif WIN32
    int iResult = ::recv(handle, recvbuf, length, 0);
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

