#pragma once
#include "Data/BinaryData.hpp"
#include "Data/Filebasic.hpp"
#include <filesystem>
#include <iostream>
#include <vector>

class World{
public:
    void create(std::filesystem::path path){
        if(!std::filesystem::exists(path)){
            std::filesystem::create_directories(path);
        }
        
        // std::vector<char> data;
        // if(readFile(path/"settings.json",data)){
        //     BinaryData bd;
        //     bd.writeBytes(data);
        //     std::u8string msg = u8"no msg";
        //     if(!bd.readString(msg)){
        //         std::cout << "oops"<<std::endl;
        //     }
        //     std::cout << (char*)msg.c_str() <<std::endl;

        // }
        
        // BinaryData bd;
        // bd.writeString(u8"{\n\t\"version\" = 1.0\n}");
        // if(!bd.writeToFile(path/"settings.json")){
        //     std::cout << "Failed to write to file" << std::endl;
        // }
    }
};
