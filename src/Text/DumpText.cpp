#include "DumpText.hpp"
#include <exception>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

FT_Library ft;
size_t freetypeCounter = 0;

void initFreetype(){
    if(freetypeCounter == 0){
        if (FT_Init_FreeType(&ft))
        {
            throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");   
        }
    }
    freetypeCounter++;
};
void deinitFreetype(){
    freetypeCounter--;
    if(freetypeCounter == 0){
        FT_Done_FreeType(ft);
    }
};

Font::Font()
{
    initFreetype();
}
Font::~Font()
{
    FT_Done_Face(face);
    deinitFreetype();
}
void Font::loadFromFile(std::filesystem::path _path){
    FT_Done_Face(face);

    std::string path = _path.string();
    if (FT_New_Face(ft, path.c_str(), 0, &face))
    {
        throw std::runtime_error("ERROR::FREETYPE: Failed to load font");
    }
    FT_Set_Pixel_Sizes(face, 0, 48);  
}
Font::Glyph Font::getGlyph(CommandPool& pool,char c){
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;  
        return {};
    }
    std::cout << (int)c << std::endl;
    int texWidth = face->glyph->bitmap.width;
    int texHeight =  face->glyph->bitmap.rows;
    int texChannels = 1;
    stbi_uc* pixels = face->glyph->bitmap.buffer;
    if(pixels == nullptr)
        return {};
    vk::DeviceSize imageSize = texWidth * texHeight * 1;

    Buffer stagingBuffer;
    stagingBuffer.createBuffer(pool.getDevice(),imageSize,vk::BufferUsageFlagBits::eTransferSrc,vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    void* data = stagingBuffer.bufferMemory.mapMemory(0, imageSize);
    memcpy(data, pixels, imageSize);
    stagingBuffer.bufferMemory.unmapMemory();
    
    image.createImage(pool,texWidth, texHeight, vk::Format::eR8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
    image.transitionImageLayout(pool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    image.copyBufferToImage(pool,stagingBuffer.buffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    image.transitionImageLayout(pool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    image.createTextureImageView(pool.getDevice(),vk::Format::eR8Srgb);
    image.createTextureSampler(  pool.getDevice());
    
    Glyph glyph{

    };
    return glyph;
}
void Text::setString(Font& font,std::string string){
    for (size_t i = 0; i < string.size(); i++){

    }
}
