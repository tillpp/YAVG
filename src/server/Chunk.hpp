#pragma once


#include <cstdint>
typedef uint16_t BlockType;
typedef uint16_t BlockData;
struct Block{
    BlockType type;
    BlockData data;
};
class Chunk{
    static const uint32_t chunkSize = 32;
    Block chunkData[chunkSize][chunkSize][chunkSize]; 
public:
    Chunk();
};