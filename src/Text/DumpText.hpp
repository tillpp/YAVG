#pragma once 
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_int2.hpp"
#include "vulkan_old/Buffer.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <ft2build.h>
#include FT_FREETYPE_H  
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan_old/Image2.hpp"
#include "Text/TexturePacker.hpp"

/*
    Can only represent ASCII characters. 
*/

class Font
{
public:
    struct Glyph{
        glm::ivec2 texPos  = glm::ivec2(0);
        glm::ivec2 texSize = glm::ivec2(0);
        glm::vec2  size;
        float advance;
        glm::vec2 bearing;
    };    
private:
    FT_Face face = nullptr;
    std::map<uint32_t,Glyph> glyphs;
public:
    TexturePacker texturePacker;

    std::map<size_t,Image> legacy;
    Image image;
    
    Font();
    Font(const Font&)=delete;
    ~Font();

    void loadFromFile(std::filesystem::path _path);

    Glyph getGlyph(CommandPool& pool,size_t frameIndex,uint32_t c);
};

class Text{
    std::u32string string;
public:
    Buffer buffer;
    size_t vertexCount;
    
    void setString(Font& font,CommandPool& pool,size_t frameIndex,std::u8string string);

};

