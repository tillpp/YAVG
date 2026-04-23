#include "SocketBase.hpp"
#include <mutex>

#ifdef WIN32
static std::mutex mutex;
struct WSA{
    WSAData data;
    WSA(){
        // Initialize Winsock
        int iResult = WSAStartup(MAKEWORD(2,2), &data);
        if (iResult != 0) {
            printf("WSAStartup failed: %d\n", iResult);
            return;
        }
    }
    ~WSA(){
        WSACleanup();
    }   
}wsa;
#endif

void SocketBase::setHandle(SOCKET handle){
    if(this->handle==handle)
        return;
    close();
    this->handle = handle;
}
SOCKET SocketBase::getHandle(){
    return handle;
}
SocketBase::SocketBase(/* args */){
}

SocketBase::~SocketBase()
{
    close();
}
void SocketBase::close(){
#ifdef __linux__
    if(handle!=INVALID_SOCKET){
        if(::close(handle)<0)
            perror("close");
    }
    handle = INVALID_SOCKET; 
#elif WIN32
    if(handle!=INVALID_SOCKET){
        if(::closesocket(handle) == SOCKET_ERROR) {
            wprintf(L"closesocket failed with error = %d\n", WSAGetLastError());
        }
    }
    handle = INVALID_SOCKET; 
#endif
    receivable = true;
    sendable   = true;
}

bool SocketBase::exist(){
    return handle != INVALID_SOCKET;
}
bool SocketBase::shutdown(bool shutdown_read,bool shutdown_write){
    if(!exist())
        return true;
    if(!shutdown_read&&!shutdown_write){
        return true;
    }
    if(shutdown_read)
        receivable = false;
    if(shutdown_write)
        sendable   = false;
    
#ifdef __linux__
    int value = 0;
    if(shutdown_read)
        value = SHUT_RD;
    else if(shutdown_write)
        value = SHUT_WR;
    else if(shutdown_read && shutdown_write)
        value = SHUT_RDWR;

    int iResult = ::shutdown(handle, value);
    if (iResult<0) {
       perror("shutdown");
       close();
       return false;
    }
#elif WIN32
    int value = 0;
    if(shutdown_read)
        value = SD_RECEIVE;
    else if(shutdown_write)
        value = SD_SEND;
    else if(shutdown_read && shutdown_write)
        value = SD_BOTH;
    if (::shutdown(handle, SD_SEND) == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        close();
        return false;
    }
#endif
    return true;
}


