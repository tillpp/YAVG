#pragma once
#include <filesystem>

// wonderful namespace polution.
namespace fs = std::filesystem;

class GameFolder
{
    void findDirectory();
public:
    fs::path directory;
    GameFolder(/* args */);
    ~GameFolder();
};

