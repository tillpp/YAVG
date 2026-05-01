#pragma once
#include <vector>
#include <filesystem>

bool readFile(const std::filesystem::path& filename,std::vector<char>& data);
std::vector<char> readFileOrThrow(const std::filesystem::path& filename);
bool writeFile(const std::filesystem::path& filename,std::vector<char> data);
void writeFileOrThrow(const std::filesystem::path& filename,std::vector<char> data);
