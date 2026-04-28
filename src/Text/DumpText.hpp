#pragma once 
#include <iostream>
#include <filesystem>
#include <ft2build.h>
#include FT_FREETYPE_H  
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan_old/Image.hpp"
#include "Text/TexturePacker.hpp"

/*
    Can only represent ASCII characters. 
*/
class Font
{
    FT_Face face = nullptr;
public:
    TexturePacker texturePacker;

    Image image;
    
    Font();
    Font(const Font&)=delete;
    ~Font();

    void loadFromFile(std::filesystem::path _path);

    struct Glyph{

    };
    Glyph getGlyph(CommandPool& pool,char c);
};

class Text{
    std::u32string string;
public:
    
    void setString(Font& font,std::u8string string);

};

