#include "TcpListener.hpp"
#include <iostream>
#include <cstring>
#include <locale>
#include <string>
#include <codecvt>

bool TcpListener::listen(std::u8string port){
    close();
#if __linux__
    struct addrinfo *result = NULL, *ptr = NULL, hints;

    memset(&hints,0,sizeof(hints));
    hints.ai_family   = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
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
        //be able to rebind to 
        #if __linux__
        {
            int on = 1;
	        setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (const void*) &on, (socklen_t) sizeof(on));
        }
        #endif
        // Setup the TCP listening socket
        if (::bind(handle, rp->ai_addr, (int)rp->ai_addrlen) == 0)
            break;                  /* Success */

        //close socket again
        if(::close(handle)<0)
            perror("close");
        handle = INVALID_SOCKET;
    }
    freeaddrinfo(result);           
    if (handle == INVALID_SOCKET) {               /* No address succeeded */
        perror("socket(),bind()\n");
        return false;
    }

    // backlog parameter, maximum length of the queue of pending connections to accept
    if ( ::listen( handle, SOMAXCONN ) < 0 ) {
        perror( "Listen");
        close();
        return false;
    }
    return true;
#elif WIN32
    struct addrinfoW *result = NULL, hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
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
            break;                  /* Success */

        //close socket again
        if(::closesocket(handle)<0)
            perror("close");
        handle = INVALID_SOCKET;
    }
    FreeAddrInfoW(result);           
    if (handle == INVALID_SOCKET) {               /* No address succeeded */
        printf("socket(),bind(): %ld\n", WSAGetLastError());
        return false;
    }

    // backlog parameter, maximum length of the queue of pending connections to accept
    if ( ::listen( handle, SOMAXCONN ) == SOCKET_ERROR ) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        close();
        return false;
    }
    return true;
#endif
}
bool TcpListener::accept(TcpSocket& client,std::string& ipAddr,int& port){
#ifdef __linux__
    // Accept a client socket
    struct sockaddr_in6 sa = {0};
    socklen_t socklen = sizeof sa;

    int clientSocket = ::accept(handle, (struct sockaddr *)&sa, &socklen);
    if (clientSocket == INVALID_SOCKET) {        
            perror("accept");
            return false;
    }
    client.setHandle(clientSocket);

    //query ip and port
    {
        char ipstr[INET6_ADDRSTRLEN];
        // deal with both IPv4 and IPv6:
        if (sa.sin6_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&sa;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
        } else { // AF_INET6
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
        }
        ipAddr = ipstr;
    }
    return true;
#elif WIN32
    // Accept a client socket
    struct sockaddr_in6 sa = {0};
    socklen_t socklen = sizeof sa;

    SOCKET clientSocket = ::accept(handle, (struct sockaddr *)&sa, &socklen);
    if (clientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        return false;
    }
    client.setHandle(clientSocket);

    //query ip and port
    {
        char ipstr[INET6_ADDRSTRLEN];
        // deal with both IPv4 and IPv6:
        if (sa.sin6_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&sa;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
        } else { // AF_INET6
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
        }
        ipAddr = ipstr;
    }
    return true;
#endif
}