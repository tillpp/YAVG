

#include "client/GameFolder.hpp"
#include "vulkan/Window.hpp"
#include "vulkan/ValidationLayer.hpp"
#include "vulkan/GraphicsQueue.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/CommandBuffer.hpp"
#include "vulkan_old/DescriptorSetLayout.hpp"
#include "vulkan/RenderSync.hpp"
#include "client/Camera.hpp"
#include "server/Region.hpp"
#include <thread>

#include "client/Game.hpp"
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "client/MeshWeaver.hpp"
#include "FastNoiseLite.h"
#include "vulkan_old/PushContant.hpp"

const std::vector<Vertex> vertices = {
    {{0,   0,0.f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{1.f, 0,0.f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{1.f, 0,1.f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{0,   0,1.f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};
const std::vector<uint16_t> indices = {
    0, 3, 2, 2, 1, 0};

class GuiSystem{
public:
    Buffer vertexBuffer, indexBuffer;
    Pipeline pipeline;
    Image image,image2;
    DescriptorSetLayout dsLayout,dsLayout2;
    PushConstant pushConstant;
    void create(Device& device,CommandPool& pool,Swapchain& swapchain,RenderSync& render,DescriptorSetLayout& _dsLayout,std::filesystem::path projectBaseDir,DepthBuffer& depthBuffer,UBO& ubo){
        image.create(pool,projectBaseDir/"assets"/"SingleplayerBtn.png");
        image2.create(pool,projectBaseDir/"assets"/"MultiplayerBtn.png");
        dsLayout.create(device,
            {
                vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
                vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
            },
            render.MAX_FRAMES_IN_FLIGHT,
            ubo,image
        );
        dsLayout2.create(device,
            {
                vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
                vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
            },
            render.MAX_FRAMES_IN_FLIGHT,
            ubo,image2
        );
        pushConstant.create();
        pipeline.create(device,projectBaseDir/"bin"/"gui.spv",
            "vertMain","fragMain",swapchain,dsLayout,depthBuffer,false,&pushConstant
        );
        vertexBuffer.createVertexBuffer(pool,vertices);
        indexBuffer.createIndexBuffer(pool,indices);

        
    }
    void draw(Window& window,CommandBuffer& buffer,RenderSync& render){
        auto& commandBuffer = buffer.commandBuffer;

        pushConstant.use(buffer,pipeline,PushConstantBlock(glm::vec2(1920,1080),glm::vec2(660+150,300),glm::vec2(300,100)));
        
        dsLayout.use(commandBuffer,render,pipeline);
        commandBuffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
        commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint16);
        commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0,0);
        
        pushConstant.use(buffer,pipeline,PushConstantBlock(glm::vec2(1920,1080),glm::vec2(660+150,450),glm::vec2(300,100)));

        dsLayout2.use(commandBuffer,render,pipeline);
        commandBuffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
        commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint16);
        commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0,0);
    }
};
struct Chunk2{
    Buffer vertexBuffer,indexBuffer;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    void create(Device& device,CommandPool& pool,FastNoiseLite& noise,size_t xOffset,size_t yOffset,size_t zOffset){
        MeshWeaver mw;
        {
            auto t1 = std::chrono::steady_clock::now();
            char* data = new char[33*33*33];
            for (size_t x = 0; x < 33; x++){
                for (size_t y = 0; y < 33; y++){
                    for (size_t z = 0; z < 33; z++){
                        int value = (noise.GetNoise((float)x+xOffset, (float)z+zOffset)*32+32)>(float) y+yOffset;
                        // if(x==0||y==0||z==0||x==32||y==32||z==32)
                        // value = 0;
                        data[x*33*33+y*33+z] = value;
                    }
                }
            }
            
            auto t2 =  std::chrono::steady_clock::now();
            mw.create(data,xOffset,yOffset,zOffset);
            std::cout <<"mesh generation time:"<< std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()<<"µs" <<std::endl;
        }
        vertices = *(std::vector<Vertex>*)&mw.vertices; // me being a bad boy. Because i am lazy.
        indices = mw.index;

        if(indices.size()){
            vertexBuffer.createVertexBuffer(pool,vertices);
            indexBuffer.createIndexBuffer(pool,indices);
        }
    }
    void draw(CommandBuffer& buffer){
        if(indices.size()){
            auto& commandBuffer = buffer.commandBuffer;
            commandBuffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
            commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint16);
            commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0,0);
        }
    }
};
void game(Game& _game,std::filesystem::path projectBaseDir) {
    time_t t;
    time(&t);
    srand(t);
    FastNoiseLite noise(t);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFrequency(0.005);
    //noise.SetFractalGain(0.75);
    noise.SetFractalOctaves(5);
    //noise.SetFractalLacunarity(2);    



    Image image;
    UBO ubo;
    DescriptorSetLayout dsLayout;
    Camera camera;
    DepthBuffer depthBuffer;
    Pipeline pipeline;
    
    image.create(_game.commandPool,projectBaseDir/"assets"/"texture.jpg");
    ubo.create(_game.device,_game.render.MAX_FRAMES_IN_FLIGHT);
    dsLayout.create(_game.device,
        {
            vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
            vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
        },
        _game.render.MAX_FRAMES_IN_FLIGHT,
        ubo,image
    );
    depthBuffer.create(_game.commandPool,_game.swapchain);
    pipeline.create(_game.device,
        projectBaseDir/"bin"/"slang.spv",
        "vertMain","fragMain",
        _game.swapchain, dsLayout,depthBuffer);

    const size_t range = 5;
    Chunk2 chunk[range][range][range];
    for (size_t x = 0; x < range; x++){
        for (size_t y = 0; y < range; y++){
            for (size_t z = 0; z < range; z++){
                chunk[x][y][z].create(_game.device,_game.commandPool,noise,x*32,-32+y*32,z*32);
            }
        }
        
    }

    GuiSystem gs;
    gs.create(_game.device,_game.commandPool,_game.swapchain,_game.render,dsLayout,projectBaseDir,depthBuffer,ubo);

    // TODO refactor the following in the future:
    auto recordCommandBuffer = [&](CommandBuffer& CB,uint32_t frameIndex,uint32_t imageIndex)
    {
        CB.begin();
        _game.swapchain.beginRendering(CB,imageIndex,&depthBuffer);
        
        float aspectRatio = static_cast<float>(_game.swapchain.swapChainExtent.width) / static_cast<float>(_game.swapchain.swapChainExtent.height);

        bool zoom = glfwGetKey(_game.window,GLFW_KEY_C) == GLFW_PRESS;
        ubo.updateUniformBuffer(frameIndex,aspectRatio, zoom,camera.pos,camera.forward);

        auto& commandBuffer = CB.commandBuffer;
        {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);
            commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_game.swapchain.swapChainExtent.width), static_cast<float>(_game.swapchain.swapChainExtent.height), 0.0f, 1.0f));
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _game.swapchain.swapChainExtent));
            
            //TODO: learn more about dynamic descriptors
            dsLayout.use(commandBuffer,_game.render,pipeline);
    
            
            for (size_t x = 0; x < range; x++){
                for (size_t y = 0; y < range; y++){
                    for (size_t z = 0; z < range; z++){
                        chunk[x][y][z].draw(CB);
                    }
                }
            }
        }
        {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *gs.pipeline.graphicsPipeline);
            commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_game.swapchain.swapChainExtent.width), static_cast<float>(_game.swapchain.swapChainExtent.height), 0.0f, 1.0f));
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _game.swapchain.swapChainExtent));
            gs.draw(_game.window,CB,_game.render);

        }
        
        
        _game.swapchain.endRendering(CB,imageIndex);
        CB.end();
    };


    


    //FPS counter
    auto lastSecond = std::chrono::steady_clock::now();
    size_t frames = 0;
    auto lastFrame = std::chrono::steady_clock::now();

    while(_game.window.update()){
        glfwPollEvents();
        //drawing
        {
            ImageIndex imageIndex;
            if(!_game.render.begin(_game.window,_game.swapchain,_game.commandPool,&depthBuffer,imageIndex))
                continue;
            auto frameIndex = _game.render.getFrameIndex();
            auto& CB        = _game.render.getCommandBuffer();
            
            recordCommandBuffer(CB,frameIndex,imageIndex);
            _game.render.end(_game.window,_game.swapchain,_game.commandPool,&depthBuffer,imageIndex);
        }
        
        //FRAMERATE STUFF
        auto now = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now-lastSecond).count()>=1000){
            float frameTime = std::chrono::duration<float, std::chrono::microseconds::period>(now - lastFrame).count();
            std::cout << "[FPS]" << frames <<" "<< frameTime <<"µs"<< std::endl;
            frames = 0;
            lastSecond = now;
        }
        frames++;
        float delta;
        {
            {
                const int FPSLimit = 120;
                auto now = std::chrono::steady_clock::now();
                float delta = std::chrono::duration<float, std::chrono::seconds::period>(now - lastFrame).count();
                if(delta < 1.f/FPSLimit){
                    float waittime = 1.f/FPSLimit-delta;
                    std::this_thread::sleep_for(std::chrono::milliseconds((int)(waittime*1000)));
                }
            }

            auto currentTime = std::chrono::steady_clock::now();
            delta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrame).count();
            lastFrame = currentTime;
        }
        //camera 
        if(_game.window.grabMouse)
        {
            camera.update(_game.window,delta);   
        }
        
    }
    _game.device.device.waitIdle();
}

int main(int argc, char const *argv[]){
    auto projectBaseDir = std::filesystem::canonical(argv[0]).parent_path().parent_path().parent_path();
    try{
        Game _game;
        game(_game,projectBaseDir);

    } catch (const vk::SystemError& err){
        std::cerr << "Vulkan error: " << err.what() << std::endl;
        return 1;
    }catch (const std::exception& err){
        std::cerr << "Error: " << err.what() << std::endl;
        return 1;
    }
    return 0;
}
