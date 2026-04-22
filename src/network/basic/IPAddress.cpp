#include "IPAddress.hpp"
#include <memory.h>
#include <assert.h>

void IPAddress::toString(std::string& ipAddr,int& port){
    //query ip and port
    {
        char ipstr[INET6_ADDRSTRLEN];
        // deal with both IPv4 and IPv6:
        if (addr.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&addr;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
        } else { // AF_INET6
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
        }
        ipAddr = ipstr;
    }
}
IPAddress::IPAddress(std::u8string ip,int port){
    memset( &addr,0, sizeof(addr) );
    if(inet_pton(AF_INET,(const char*)ip.c_str(),&addr)==1){
        sockaddr_in* a = (sockaddr_in*)&addr ;
        a->sin_family = AF_INET;
        a->sin_port   = htons(port);
    }else if(inet_pton(AF_INET6,(const char*)ip.c_str(),&addr)==1){
        sockaddr_in6* a = (sockaddr_in6*)&addr;
        a->sin6_family = AF_INET6;
        a->sin6_port   = htons(port);
    }else{
        assert(0 && "unreachable");
    }           
}
