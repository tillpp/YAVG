#pragma once
#include <filesystem>
#include "network/basic/TcpListener.hpp"


class Server
{
public:
    void create(std::filesystem::path worldPath){
        if(!std::filesystem::exists(worldPath)){
            std::filesystem::create_directories(worldPath);
        }
        // TcpListener tcpl;
        // tcpl.listen(u8"5555");
        // TcpSocket client;
        // std::string ipAddr;
        // int port;
        // tcpl.accept(client,ipAddr,port);
        // char buffer[255*255];
        // size_t received;
        // client.recv(buffer,255*255,received);
        // std::cout << ">" <<std::string(buffer,buffer+received)<<std::endl;
        // client.send(buffer,received,received);
    }
};
