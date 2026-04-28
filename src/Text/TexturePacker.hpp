#pragma once
#include <glm/glm.hpp>
#include <limits>

/*
    Keeps track of empty space on a texture atlas 
*/

class TexturePacker
{
    struct Node{
        bool direction;// 0 = x, 1 = y
        glm::ivec2 offset = {0,0}; // upper left corner
        glm::ivec2 size = glm::ivec2(std::numeric_limits<int>::max());
    };
    std::vector<Node> nodes;
    glm::ivec2 size;

    static glm::ivec2 nearestPowerTwoSize(glm::ivec2 size){
        glm::ivec2 rv(1,1);
        while(size.x>=rv.x||size.y>=rv.y)
            rv *= 2;
        return rv;
    }
public:

    TexturePacker(glm::ivec2 size);
    ~TexturePacker();

    void setSize(glm::ivec2 size);
    
    struct Response{
        glm::ivec2 position;
        glm::ivec2 newSize;
    };
    /// @brief request a position in a texture to draw something.
    /// @param glyphSize
    /// @return 
    Response request(glm::ivec2 glyphSize);
};

