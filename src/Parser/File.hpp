#pragma once
#include <vector>
#include <filesystem>
#include <fstream>

std::vector<char> readFile(const std::filesystem::path& filename);
void writeFile(std::vector<char> data,const std::filesystem::path& filename);
