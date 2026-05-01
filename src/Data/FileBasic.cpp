#include "Filebasic.hpp"
#include <cstdio>
#include <fstream>

bool readFile(const std::filesystem::path& filename,std::vector<char>& data){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    data.insert(data.end(), buffer.begin(), buffer.end());
    return true;
}
std::vector<char> readFileOrThrow(const std::filesystem::path& filename){
    std::vector<char> data;
    if(!readFile(filename,data)){
        throw std::runtime_error("failed to read file :"+filename.string());
    }
    return data;
}
bool writeFile(const std::filesystem::path& filename,std::vector<char> data){
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.write(data.data(), static_cast<std::streamsize>(data.size()));
    file.close();
    return true;
}
void writeFileOrThrow(std::vector<char> data,const std::filesystem::path& filename){
    if(!writeFile(filename,data)){
        throw std::runtime_error("failed to write file :"+filename.string());
    }
}
