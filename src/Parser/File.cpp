#include "File.hpp"

std::vector<char> readFile(const std::filesystem::path& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}
void writeFile(std::vector<char> buffer,const std::filesystem::path& filename){
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    file.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
}
