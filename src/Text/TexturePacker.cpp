#include "TexturePacker.hpp"

TexturePacker::TexturePacker(glm::ivec2 size):size(size){
    nodes.push_back((Node){
        .direction = true,
        .offset    = {0,0},
        .size      = glm::ivec2(std::numeric_limits<int>::max())
    });
}

TexturePacker::~TexturePacker()
{
}

TexturePacker::Response TexturePacker::request(glm::ivec2 glyphSize){
    Response res{
        .position = size,
        .newSize  = size,
    };
    struct{
        int difference; // transversal size difference
        size_t index;
    }candidate;
    candidate.index      = nodes.size();
    candidate.difference = std::numeric_limits<int>::max();
    do{
        for (size_t i = 0; i < nodes.size(); i++){
            auto& node = nodes[i];
            //does it fit in node?
            if(glyphSize.x>node.size.x || glyphSize.y>node.size.y)
                continue;
            //does it fit in textureAtlas? 
            glm::ivec2 endPosition = node.offset+glyphSize;
            if(endPosition.x>size.x || endPosition.y>size.y)
                continue;
            //transversal difference smaller? take this candidate
            auto transversalDifference = node.size[!node.direction]-glyphSize[!node.direction];
            if(transversalDifference<candidate.difference){
                candidate.difference = transversalDifference;
                //store as candidate.index
                candidate.index      = i; 
            }
        }
        //failed?
        if(candidate.index==nodes.size()){
            //try again
            size = nearestPowerTwoSize(size);
        }else break;
    }while(true);
     Node node = nodes[candidate.index];
    nodes.erase(nodes.begin()+candidate.index);
    //create two new nodes:    transversal and longitudinal      
    Node transversal;
    transversal.direction = !node.direction;
    transversal.offset = node.offset;
    transversal.offset[!node.direction] += glyphSize[!node.direction];
    transversal.size[node.direction]  = glyphSize[node.direction];
    transversal.size[!node.direction] = node.size[!node.direction]-glyphSize[!node.direction];
    Node longitudinal;
    longitudinal.direction = node.direction;
    longitudinal.offset = node.offset;
    longitudinal.offset[node.direction] += glyphSize[node.direction];
    longitudinal.size[node.direction] = node.size[node.direction]-glyphSize[node.direction];
    longitudinal.size[!node.direction]= node.size[!node.direction];
    
    nodes.push_back(transversal );
    nodes.push_back(longitudinal);
    
    res.position = node.offset;
    res.newSize  = size;
    return res;
}
glm::ivec2 TexturePacker::getSize()const{
    return size;
}
