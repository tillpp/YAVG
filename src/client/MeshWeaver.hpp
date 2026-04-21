#pragma once
#include "../vulkan_old/Device.hpp"
#include <glm/glm.hpp>
#include <iostream>

class MeshWeaver
{
public:

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static vk::VertexInputBindingDescription getBindingDescription()
        {
            return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
        }
        static std::array<vk::VertexInputAttributeDescription,3> getAttributeDescriptions()
        {
        return {{{.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
                {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
                {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)}}};
        }
    };

    static constexpr int chunkSize = 32;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> index;

    void create(char chunk[(chunkSize+1)*(chunkSize+1)*(chunkSize+1)]){
        auto getBlock = [&](int x,int y,int z)->int
        {
            int index = x*(chunkSize+1)*(chunkSize+1)+y*(chunkSize+1)+z;
            return chunk[index];
        };
        //TODO: this can be optimized cause it only stores a bool:
        //TODO: dont recreate it for every chunk. 
        size_t vertexIndexSize = (chunkSize+1)*(chunkSize+1)*(chunkSize+1);
        int32_t vertexIndex[vertexIndexSize];
        for (size_t i = 0; i < vertexIndexSize; i++){
            vertexIndex[i] = -1;
        }
        auto getVertexIndex = [&](int x,int y,int z)->int
        {
            int index = x*(chunkSize+1)*(chunkSize+1)+y*(chunkSize+1)+z;
            if(vertexIndex[index] == -1){
                vertexIndex[index] = vertices.size();
                float scale = 1.0/32;
                vertices.push_back(Vertex{
                    .pos = glm::vec3(x,y,z),
                    .color = scale*glm::vec3(x,y,z),
                    .texCoord = glm::vec2(1,1)
                });
            }
            return vertexIndex[index];
        };

        auto createFace = [&](int tmpIndex[4],bool lookFromPositive)
        {
            if(lookFromPositive){
                std::vector<unsigned char> order{0, 1, 2, 2, 3, 0};
                for (auto &&i : order){
                    index.push_back(tmpIndex[i]);
                }
            }else{
                std::vector<unsigned char> order{0, 3, 2, 2, 1, 0};
                for (auto &&i : order){
                    index.push_back(tmpIndex[i]);
                }
            }
        };
        auto createBlockMesh = [&](int x,int y,int z){
            int tmpIndex[4]; // looking from positive direction clockwise. 
            bool lookFromPositive;
            //right
            if(getBlock(x,y,z)!=getBlock(x+1,y,z)){
                tmpIndex[0] = getVertexIndex(x+1,y,z);
                tmpIndex[1] = getVertexIndex(x+1,y+1,z);
                tmpIndex[2] = getVertexIndex(x+1,y+1,z+1);
                tmpIndex[3] = getVertexIndex(x+1,y,z+1);
                lookFromPositive = (getBlock(x+1,y,z) == 0);
                createFace(tmpIndex,lookFromPositive);
            }
            //top
            if(getBlock(x,y,z)!=getBlock(x,y+1,z)){
                tmpIndex[0] = getVertexIndex(x,  y+1,z);
                tmpIndex[1] = getVertexIndex(x,  y+1,z+1);
                tmpIndex[2] = getVertexIndex(x+1,y+1,z+1);
                tmpIndex[3] = getVertexIndex(x+1,y+1,z);
                lookFromPositive = (getBlock(x,y+1,z) == 0);
                createFace(tmpIndex,lookFromPositive);
            }
            //behind
            if(getBlock(x,y,z)!=getBlock(x,y,z+1)){
                tmpIndex[0] = getVertexIndex(x,  y,  z+1);
                tmpIndex[1] = getVertexIndex(x+1,y,  z+1);
                tmpIndex[2] = getVertexIndex(x+1,y+1,z+1);
                tmpIndex[3] = getVertexIndex(x,  y+1,z+1);
                lookFromPositive = (getBlock(x,y,z+1) == 0);
                createFace(tmpIndex,lookFromPositive);
            }
        };

        for (size_t x = 0; x < chunkSize; x++){
            for (size_t y = 0; y < chunkSize; y++){
                for (size_t z = 0; z < chunkSize; z++){
                    createBlockMesh(x,y,z);
                }
            }   
        }
        
    }
};