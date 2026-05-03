#pragma once 
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_int2.hpp"
#include "vulkan/setup/CommandBuffer.hpp"
#include "vulkan/setup/RenderSync.hpp"
#include "vulkan_old/Buffer.hpp"
#include "vulkan_old/Image.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <ft2build.h>
#include <memory>
#include <vector>
#include FT_FREETYPE_H  
#include "vulkan/setup/CommandPool.hpp"
#include "Text/TexturePacker.hpp"

/*
    Can only represent non-CTL characters. 
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

    std::shared_ptr<Image> image;
    
    Font();
    Font(const Font&)=delete;
    ~Font();

    void loadFromFile(std::filesystem::path _path);

    Glyph getGlyph(RenderSync* render,CommandPool& pool,uint32_t c);
};

class Text{
    
    struct VertexBuffer:public Buffer{
        
        size_t vertexCount;
    };
    std::shared_ptr<VertexBuffer> buffer;
public:
    std::u32string string;
    double width;

    void setString(Font& font,CommandPool& pool,RenderSync* render,std::u32string string);
    void setString(Font& font,CommandPool& pool,RenderSync* render,std::u8string string);
    
    void draw(CommandBuffer& CB){
        CB.commandBuffer.bindVertexBuffers(0, *buffer->buffer, {0});
        CB.commandBuffer.draw(static_cast<uint32_t>(buffer->vertexCount), 1, 0, 0);
    }
};

