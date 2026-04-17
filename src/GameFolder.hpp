#pragma once
#include <filesystem>

// wonderful namespace polution.
namespace fs = std::filesystem;

class GameFolder
{
    void findDirectory();

    fs::path directory;
public:
    GameFolder(/* args */);
    ~GameFolder();
};

