#pragma once
#include <filesystem>

class Server
{
public:
    void create(std::filesystem::path worldPath){
        if(!std::filesystem::exists(worldPath)){
            std::filesystem::create_directories(worldPath);
        }
        
    }
};
