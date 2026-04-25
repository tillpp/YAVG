#pragma once 
#include <string>
#include <assert.h>
#include "Byteswap.hpp"
#include "File.hpp"


class BinaryData
{    
    template<class T>
    bool read(T& t){
        if(content.size() < i+sizeof(T))
            return false;
        void* data = &t;
        memcpy(data,content.data()+i,sizeof(T));
        if(std::endian::native == std::endian::little)
            t = byteswap(t);    
        i+= sizeof(T);
        return true;
    }

    template<class T>
    void write(T t){
        if(std::endian::native == std::endian::little)
            t = byteswap(t);    
        char* data = (char*)&t;
        content.insert(content.end(),data,data+sizeof(T));
    }
    
    std::vector<char> content;
    size_t i = 0;
public:
    void writeToFile(std::filesystem::path path){
        writeFile(content,path);      
    }
    void readFromFile(std::filesystem::path path){
        readBytes(readFile(path));        
    }
    void readBytes(std::vector<char> s){
        content.insert(content.end(),s.begin(),s.end());
    }

    bool readI8 (int8_t& t){
        return read<int8_t>(t);
    }        
    bool readI16(int16_t& t){
        return read<int16_t>(t);
    }        
    bool readI32(int32_t& t){
        return read<int32_t>(t);
    }        
    bool readI64(int64_t& t){
        return read<int64_t>(t);
    }
    bool readU8 (uint8_t& t){
        return read<uint8_t>(t);
    }        
    bool readU16(uint16_t& t){
        return read<uint16_t>(t);
    }        
    bool readU32(uint32_t& t){
        return read<uint32_t>(t);
    }        
    bool readU64(uint64_t& t){
        return read<uint64_t>(t);
    }
    bool readf32(float& t){
        return read<float>(t);
    }
    bool readf64(double& t){
        return read<double>(t);
    }
    

    void writeI8 (int8_t data){
        write<int8_t>(data);
    }        
    void  writeI16(int16_t data){
        write<int16_t>(data);
    }        
    void  writeI32(int32_t data){
        write<int32_t>(data);
    }        
    void  writeI64(int64_t data){
        write<int64_t>(data);
    }
    void  writeU8 (uint8_t data){
        write<uint8_t>(data);
    }        
    void writeU16(uint16_t data){
        write<uint16_t>(data);
    }        
    void writeU32(uint32_t data){
        write<uint32_t>(data);
    }        
    void writeU64(uint64_t data){
        write<uint64_t>(data);
    }
    void    writef32(float data){
        write<float>(data);
    }
    void   writef64(double data){
        write<double>(data);
    }
};