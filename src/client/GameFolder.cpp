#include "GameFolder.hpp"
#include <iostream>

GameFolder::GameFolder(/* args */)
{
    findDirectory();

}

GameFolder::~GameFolder()
{
}

void GameFolder::findDirectory(){
    std::string folder = "YetAnotherVoxelGame";
#ifdef _WIN32
    auto path = std::getenv("APPDATA");
    if(path){
        directory = fs::path(path)/folder;
    }
#elif __APPLE__
    auto path = std::getenv("HOME");
    if(path){
        directory = fs::path(path)/"Library"/"Application Support"/folder;
    }
#else 
    auto path = std::getenv("XDG_DATA_HOME");
    if(path){
        directory = fs::path(path)/folder;
    }else{
        path = std::getenv("HOME");
        if(path){
            directory = fs::path(path)/".local"/"share"/folder;
        }
    }
#endif
    std::cout << directory.c_str() << std::endl;
    if( !directory.empty() && !fs::exists(directory)){
        fs::create_directories(directory);
    }
}