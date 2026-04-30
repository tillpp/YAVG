#include "DumpText.hpp"
#include "Text/TexturePacker.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/vector_int2.hpp"
#include "vulkan/setup/CommandBuffer.hpp"
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan_old/Buffer.hpp"
#include <cstdint>
#include <exception>
#include <utility>
#include <vector>
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

Font::Font():texturePacker(glm::ivec2(0,0))
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
Font::Glyph Font::getGlyph(CommandPool& pool,size_t frameIndex,uint32_t c){
    if(glyphs.contains(c))
        return glyphs[c];
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;  
        return {};
    }
    
    int texWidth = face->glyph->bitmap.width;
    int texHeight =  face->glyph->bitmap.rows;
    int texChannels = 1;
    stbi_uc* pixels = face->glyph->bitmap.buffer;
    if(!pixels){
        Glyph glyph{
            .texPos  = glm::ivec2(0),
            .texSize = glm::ivec2(0),
            .size   = glm::ivec2(face->glyph->metrics.width/64.f,face->glyph->metrics.height/64.f),
            .advance = face->glyph->advance.x/64.f,
            .bearing = glm::vec2(face->glyph->metrics.horiBearingX,face->glyph->metrics.horiBearingY)/64.f,
        };
        glyphs[c] = glyph;
        return  glyph;
    }
    vk::DeviceSize imageSize = texWidth * texHeight * 1;

    Buffer stagingBuffer;
    stagingBuffer.createBuffer(pool.getDevice(),imageSize,vk::BufferUsageFlagBits::eTransferSrc,vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    void* data = stagingBuffer.bufferMemory.mapMemory(0, imageSize);
    memcpy(data, pixels, imageSize);
    stagingBuffer.bufferMemory.unmapMemory();
    
    auto oldSize = texturePacker.getSize();
    auto resp = texturePacker.request(glm::ivec2(texWidth,texHeight));
    if(oldSize != resp.newSize){
        Image newImage;
        newImage.createImage(pool,resp.newSize.x, resp.newSize.y, vk::Format::eR8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
        newImage.transitionImageLayout(pool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        // copy the old textureAtlas
        if(oldSize.x != 0 && oldSize.y != 0){

            CommandBuffer cb(pool);
            cb.beginSingleTimeCommands();
            cb.transition_image_layout(
                image.image, 
                vk::ImageLayout::eShaderReadOnlyOptimal, 
                vk::ImageLayout::eTransferSrcOptimal, 
                vk::AccessFlags2::BitsType::eShaderRead, 
                vk::AccessFlags2::BitsType::eTransferRead,
                vk::PipelineStageFlags2::BitsType::eFragmentShader, 
                vk::PipelineStageFlags2::BitsType::eTransfer, 
                vk::ImageAspectFlags::BitsType::eColor
            );
            cb.commandBuffer.copyImage(
                image.image, 
                vk::ImageLayout::eTransferSrcOptimal,
                newImage.image, 
                vk::ImageLayout::eTransferDstOptimal, {
                vk::ImageCopy{
                    .srcSubresource = { vk::ImageAspectFlagBits::eColor, 0,0,1},
                    .srcOffset = { 0,0,0},
                    .dstSubresource = { vk::ImageAspectFlagBits::eColor, 0,0,1},
                    .dstOffset = { 0,0,0},
                    .extent = { (uint32_t)oldSize.x,(uint32_t)oldSize.y,1},
                }
            });
            cb.transition_image_layout(
                image.image, 
                vk::ImageLayout::eTransferSrcOptimal, 
                vk::ImageLayout::eShaderReadOnlyOptimal, 
                vk::AccessFlags2::BitsType::eTransferRead, 
                vk::AccessFlags2::BitsType::eShaderRead,
                vk::PipelineStageFlags2::BitsType::eTransfer, 
                vk::PipelineStageFlags2::BitsType::eFragmentShader, 
                vk::ImageAspectFlags::BitsType::eColor
            );
            cb.endSingleTimeCommands(pool);
        }
        newImage.transitionImageLayout(pool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        newImage.createTextureImageView(pool.getDevice(),vk::Format::eR8Srgb);
        {
             vk::PhysicalDeviceProperties properties = pool.getDevice().physicalDevice.getProperties();
            vk::SamplerCreateInfo samplerInfo{
                .magFilter = vk::Filter::eNearest, 
                .minFilter = vk::Filter::eNearest,  
                .mipmapMode = vk::SamplerMipmapMode::eNearest,
                .addressModeU = vk::SamplerAddressMode::eRepeat,
                .addressModeV = vk::SamplerAddressMode::eRepeat,
                .addressModeW = vk::SamplerAddressMode::eRepeat,
                .anisotropyEnable = vk::False, // TODO: deactive when .samplerAnisotropy isnt available and make maxAnisotropy = 1.0f
                .maxAnisotropy = 0, 
                .compareEnable = vk::False,
                .compareOp = vk::CompareOp::eAlways
            };
            newImage.textureSampler = vk::raii::Sampler(pool.getDevice().device, samplerInfo);
        }
        legacy[frameIndex] = std::move(image);
        image = std::move(newImage);

    }

    //copy the new glyph
    {
        image.transitionImageLayout(pool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        CommandBuffer commandBuffer(pool);
        commandBuffer.beginSingleTimeCommands();
        vk::BufferImageCopy region{ 
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, 
            .imageOffset = {resp.position.x, resp.position.y, 0}, 
            .imageExtent = {(uint32_t)texWidth, (uint32_t)texHeight, 1} 
        };
        commandBuffer.commandBuffer.copyBufferToImage(stagingBuffer.buffer,image.image,vk::ImageLayout::eTransferDstOptimal,{region});
        commandBuffer.endSingleTimeCommands(pool);
        image.transitionImageLayout(pool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    }        
    
    Glyph glyph{
        .texPos  = glm::ivec2(resp.position),
        .texSize = glm::ivec2(texWidth,texHeight),
        .size   = glm::ivec2(face->glyph->metrics.width/64.f,face->glyph->metrics.height/64.f),
        .advance = face->glyph->advance.x/64.f,
        .bearing = glm::vec2(face->glyph->metrics.horiBearingX,face->glyph->metrics.horiBearingY)/64.f,
    };
    glyphs[c] = glyph;
    return glyph;
}
void Text::setString(Font& font,CommandPool& pool,size_t frameIndex,std::u8string string){
    std::vector<Vertex> vertices;
    std::vector<Font::Glyph> glyphs;
    for (auto& c : string) {
        auto glyph = font.getGlyph(pool, frameIndex, c);
        glyphs.push_back(glyph);
    }

    float advance = 0;
    for(auto& glyph:glyphs){   
        glm::vec2 texPos  = glm::vec2(glyph.texPos)/glm::vec2(font.texturePacker.getSize());
        glm::vec2 texEnd = glm::vec2(glyph.texSize+glyph.texPos)/glm::vec2(font.texturePacker.getSize());
        
        glm::vec2 pos = glm::vec2(glyph.bearing.x,48-glyph.bearing.y)/48.f;
        glm::vec2 size = glm::vec2(glyph.size)/48.f;
        pos.x += advance;
        glm::vec2 end = pos+size;
        
        std::vector<Vertex> tmpVertices = {
            {{pos.x, 0,pos.y}, {1.0f, 0.0f, 0.0f}, {texPos.x, texPos.y}},
            {{pos.x, 0,end.y}, {1.0f, 1.0f, 1.0f}, {texPos.x, texEnd.y}},
            {{end.x, 0,end.y}, {0.0f, 0.0f, 1.0f}, {texEnd.x, texEnd.y}},
            {{end.x, 0,end.y}, {0.0f, 0.0f, 1.0f}, {texEnd.x, texEnd.y}},
            {{end.x, 0,pos.y}, {0.0f, 1.0f, 0.0f}, {texEnd.x, texPos.y}},
            {{pos.x, 0,pos.y}, {1.0f, 0.0f, 0.0f}, {texPos.x, texPos.y}},
        };
        vertices.insert(vertices.end(), tmpVertices.begin(), tmpVertices.end());
        advance += glyph.advance/48;
    }
    buffer.createVertexBuffer(pool, vertices.data(), vertices.size());
    vertexCount = vertices.size();
}
