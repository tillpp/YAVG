

#include "GLFW/glfw3.h"
#include "client/FPSMessurement.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "network/basic/SocketPoll.hpp"
#include "network/basic/TcpListener.hpp"
#include "network/basic/TcpSocket.hpp"
#include "vulkan/DescriptorLayout.hpp"
#include "vulkan/setup/Instance.hpp"
#include "vulkan/setup/Window.hpp"
#include "vulkan/setup/Device.hpp"
#include "vulkan/setup/CommandBuffer.hpp"
#include "vulkan/setup/RenderSync.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/Descriptor.hpp"
#include "client/Camera.hpp"
#include "Text/DumpText.hpp"
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cuchar>
#include <linux/input-event-codes.h>
#include <memory>
#include <ratio>
#include <string>
#include <thread>

#include "client/Game.hpp"
#include "vulkan_old/Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "client/MeshWeaver.hpp"
#include "FastNoiseLite.h"
#include "vulkan/PushContant.hpp"
#include "vulkan/DescriptorLayout.hpp"


const std::vector<Vertex> vertices = {
    {{0,   0,0.f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{1.f, 0,0.f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{1.f, 0,1.f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{0,   0,1.f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};
const std::vector<uint16_t> indices = {
    0, 3, 2, 2, 1, 0};

FPSMessurement fpsCounter;

struct Setup{
    Instance& instance;
    Window& window;
    Device& device;
    GraphicsQueue& queue;
    Swapchain& swapchain;
    CommandPool& pool;    
    RenderSync* render;
    
    std::filesystem::path projectBaseDir; 
    DepthBuffer &depthBuffer;
    
    //current rendering
    CommandBuffer& buffer;    
};
class Screen{
public:
    virtual void create(class GuiSystem& gs,Setup& setup)=0;
    virtual ~Screen()=default;
    virtual void draw(GuiSystem& gs,Setup& setup,glm::vec2 relativeMousePosition)=0;
};
class GuiSystem{
public:
    Pipeline pipeline;
    Pipeline pipelineText;
    DescriptorSetLayout dsLayout;
    DescriptorSet dsTexture;

    struct PushConstantBlock{
        glm::vec2 position;
        glm::vec2 size;
        PushConstantBlock(glm::vec2 screenSize,glm::vec2 position,glm::vec2 size){
            this->position = position/screenSize;
            this->size     = size    /screenSize;
        }
    };
    struct PushConstantBlockText{
        glm::vec4 color;
        glm::vec2 position;
        glm::vec2 size;
        glm::ivec2 textAtlas;
        PushConstantBlockText(glm::vec2 screenSize,glm::vec2 position,glm::vec2 size,glm::ivec2 textureAtlasSize,glm::vec4 color){
            this->position = position/screenSize;
            this->size     = size    /screenSize;
            this->textAtlas = textureAtlasSize;
            this->color    = color;
        }

    };
    PushConstant pushConstant;
    PushConstant pushConstantText;
    Font font;
    std::shared_ptr<Screen> screen;

    void setScreen(Setup& setup,std::shared_ptr<Screen> newScreen){
        newScreen->create(*this,setup);
        screen = newScreen;
    }
    void create(Setup& setup);
    void draw(Setup& setup);
};
struct Server{
    TcpListener listener;
    void start(std::u32string ipAddress){
        SocketPoll poll;
        listener.listen(u8"5555");
        poll.add(listener);

        TcpSocket client;
        while(true){
            if(poll.wait()){
                if(poll.isWriteable(listener)||poll.isReadable(listener)){
                    std::string ipAddr;
                    int port;
                    listener.accept(client,  ipAddr, port);
                    poll.add(client);
                }
                if (poll.isReadable(client)||poll.isWriteable(client)) {
                
                    char buffer[1000];
                    size_t received;
                    if(client.recv(buffer, 1000, received)){
                        std::cout << std::string(buffer,buffer+received)<<std::endl;
                    }
                }
                std::cout << ".";

            }

        }
    }
};
Server server;
struct Client{
    TcpSocket socket;
    void join(std::u32string u32address){
        //convert u32 to u8string:
        std::u8string u8address;
        {
            setlocale(LC_ALL, "en_US.utf8");
            char buffer[MB_CUR_MAX];
            std::mbstate_t state{}; 
            
            for (auto& c32: u32address) {
                if(size_t rc = std::c32rtomb(buffer, c32, &state))
                {
                    if (rc == (std::size_t) - 1)
                        break;
                    if (rc == (std::size_t) - 2)
                        break;
                    u8address += std::u8string(buffer,buffer+rc);
                }
            }
        }
        size_t split = u8address.find(':');
        std::u8string ip = u8address.substr(0,split); 
        std::u8string port = u8"5555";  
        if(split!=std::string::npos){
            port = u8address.substr(split+1);
        }
        if(!socket.connect(ip, port)){
            std::cout << "failed to connect to "<<(char*)ip.c_str()<<" "<<(char*)port.c_str() << std::endl;
        }

        while (socket.exist()) {
            char buff[1000];
            size_t received;
            if(socket.recv(buff, 1000, received)){
                std::cout << std::string(buff,buff+received) <<std::endl;
            }

        }
    }
};
Client client;
class MultiplayerMenu:public Screen{
    Text ipAddressLabel;
    Text nameLabel;
    Text join;
    Text host;
    Text* selected = nullptr;
    void create(GuiSystem& gs,Setup& stp)override{
        host.setString(gs.font, stp.pool, stp.render, u8"Host");
        join.setString(gs.font, stp.pool, stp.render, u8"Join");

    }
    void draw(GuiSystem& gs,Setup& stp,glm::vec2 relativeMousePosition)override{
        auto screenSize = glm::vec2(1920,1080);
        auto mousePosition = relativeMousePosition*screenSize;

        auto& commandBuffer = stp.buffer.commandBuffer;
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *gs.pipelineText.graphicsPipeline);
        gs.dsTexture.bind(stp.device,commandBuffer,*stp.render,gs.pipelineText);

        if(!nameLabel.string.size() && &nameLabel!=selected)
            nameLabel.setString(gs.font, stp.pool, stp.render, u8"Enter name of sacrifice");
        if(!ipAddressLabel.string.size()&& &ipAddressLabel!=selected)
            ipAddressLabel.setString(gs.font, stp.pool, stp.render, u8"Enter Server Address");

        Text* texts[] = {&ipAddressLabel,&nameLabel,&join,&host};
        glm::vec2 positions[] = {glm::vec2(300,300),glm::vec2(300,420),glm::vec2(1200,900),glm::vec2(100,900)};
        glm::vec4 colors[] = {glm::vec4(0.2,0.2,0.7,1),glm::vec4(0.2,0.2,0.7,1),glm::vec4(0.5),glm::vec4(0.5)};

        if(glfwGetMouseButton(stp.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){  
            selected = nullptr;
        }
        for (int i = 0;i<4 ;i++) {
            auto& text = texts[i];
            auto position = positions[i];
            auto color = colors[i];
            glm::vec2 size(60);
            if( position.x < mousePosition.x && mousePosition.x < position.x+size.x*text->width && position.y < mousePosition.y && mousePosition.y < position.y+size.y){
                color = glm::vec4(1);
                if(glfwGetMouseButton(stp.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){  
                    if(i < 2){
                        selected = texts[i];
                        selected->string.clear();
                        stp.window.textInput.clear();
                    }else if(i==2){
                        client.join(ipAddressLabel.string);
                    }else if(i==3){
                        server.start(ipAddressLabel.string);
                    }
                }
            }
            if(texts[i]==selected){
                color = glm::vec4(1,0,0,1);
                if(stp.window.textInput.size()){
                    selected->setString(gs.font, stp.pool, stp.render, selected->string+stp.window.textInput);
                    stp.window.textInput.clear();
                }
            }
            gs.pushConstantText.use(stp.buffer,gs.pipelineText,GuiSystem::PushConstantBlockText(screenSize,position,glm::vec2(60),gs.font.texturePacker.getSize(),color));
            text->draw(stp.buffer);
        }   
    }
};
class MainMenu:public Screen{
    Text text,text2,text3,text4;
    std::u32string string;

public:
    std::chrono::steady_clock::time_point start;

    void create(GuiSystem& gs,Setup& stp)override{
        
        text.setString( gs.font, stp.pool, stp.render , u8"Suffer alone    😈");
        text2.setString(gs.font, stp.pool, stp.render, u8"Suffer together 😈 😈");
        text3.setString(gs.font, stp.pool, stp.render, u8"Exit");
        text4.setString(gs.font, stp.pool, stp.render, u8"Hz");
        
        start = std::chrono::steady_clock::now();
    }
    void draw(GuiSystem& gs,Setup& stp,glm::vec2 relativeMousePosition)override{
        auto screenSize = glm::vec2(1920,1080);
        auto mousePosition = relativeMousePosition*screenSize;

        // if(glfwGetKey(stp.window, GLFW_KEY_Y)){
            
        //     //char c = 32+rand()%96;
        //     //font.getGlyph(pool, render.getFrameIndex(), c);
            
        //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
        //     // while(glfwGetKey(window, GLFW_KEY_Y)){
        //     //     glfwWaitEvents();
        //     // }
        //     text.setString(gs.font, stp.pool, stp.render, u8"hi");
        // }
        if(stp.window.textInput.size()){
            string += stp.window.textInput;
            stp.window.textInput.clear();
            text.setString(gs.font,stp.pool, stp.render, string);
        }

        auto& commandBuffer = stp.buffer.commandBuffer;
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *gs.pipelineText.graphicsPipeline);
        gs.dsTexture.bind(stp.device,commandBuffer,*stp.render,gs.pipelineText);

        //FPS
        gs.pushConstantText.use(stp.buffer,gs.pipelineText,GuiSystem::PushConstantBlockText(screenSize,glm::vec2(0),glm::vec2(30),gs.font.texturePacker.getSize(),glm::vec4(1)));
        text4.draw(stp.buffer);
        std::string fpsString = std::to_string(fpsCounter.currentFPS)+" Hz";
        text4.setString(gs.font, stp.pool, stp.render, std::u8string(fpsString.begin(),fpsString.end()).c_str());
        
        Text* texts[] = {&text,&text2,&text3};
        
        size_t i = 0;
        for (auto& text : texts) {
            auto position = glm::vec2(660,400+i*100);
            auto size = glm::vec2(60,60);
            auto color = glm::vec4(0.5,0.5,0.5,1);
            //color = glm::vec4(0.3,0.3,0.7,1); TODO: make editable text blue
            


            if( position.x < mousePosition.x && mousePosition.x < position.x+size.x*text->width && position.y < mousePosition.y && mousePosition.y < position.y+size.y){
                color = glm::vec4(1);
                if(glfwGetMouseButton(stp.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
                    if(i==1){
                        gs.setScreen(stp, std::make_shared<MultiplayerMenu>());
                        return;
                    }
                }
                if(i==0)
                    text->setString(gs.font, stp.pool, stp.render, u8"Singleplayer");
                if(i==1)
                    text->setString(gs.font, stp.pool, stp.render, u8"Multiplayer");
            }else{
                if(i==0)
                    text->setString(gs.font, stp.pool, stp.render, u8"Suffer alone    😈");
                if(i==1)
                    text->setString(gs.font, stp.pool, stp.render, u8"Suffer together 😈 😈");

            }



            gs.pushConstantText.use(stp.buffer,gs.pipelineText,GuiSystem::PushConstantBlockText(screenSize,position,size,gs.font.texturePacker.getSize(),color));

            text->draw(stp.buffer);
            i++;
            if(i==2){
                if(std::chrono::steady_clock::now() > start+std::chrono::milliseconds(400))
                    break;
            }
        }
    } 
    ~MainMenu(){

    }
};
void GuiSystem::create(Setup& stp){

    
        dsLayout.create(stp.device,{
            DescriptorLayout(1,vk::ShaderStageFlagBits::eFragment, vk::DescriptorType::eCombinedImageSampler),
        });

        font.loadFromFile(stp.projectBaseDir/"assets"/"fonts"/"unscii-16-full.ttf");
        font.getGlyph(stp.render,stp.pool,'-');

        dsTexture.create(stp.device,stp.render,dsLayout,{
            DescriptorLayout(1,vk::ShaderStageFlagBits::eFragment, vk::DescriptorType::eCombinedImageSampler),
        });
        dsTexture.setResource(1, font.image);
        
        pushConstant.create(vk::ShaderStageFlagBits::eVertex,0,sizeof(PushConstantBlock));
        pipeline.create(stp.render,stp.device,stp.projectBaseDir/"bin"/"gui.spv",
            "vertMain","fragMain",stp.swapchain,dsLayout,stp.depthBuffer,false,&pushConstant
        );
        pushConstantText.create(vk::ShaderStageFlagBits::eVertex,0,sizeof(PushConstantBlockText));
        pipelineText.create(stp.render,stp.device,stp.projectBaseDir/"bin"/"text.spv",
            "vertMain","fragMain",stp.swapchain,dsLayout,stp.depthBuffer,false,&pushConstantText
        );
        setScreen(stp,std::make_shared<MainMenu>());
    }
void GuiSystem::draw(Setup& stp){
        stp.buffer.commandBuffer.setViewport(0, vk::Viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(stp.swapchain.swapChainExtent.width),
            .height = static_cast<float>(stp.swapchain.swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        });
        stp.buffer.commandBuffer.setScissor(0, vk::Rect2D{
            .offset = vk::Offset2D{.x = 0,.y = 0},
            .extent = stp.swapchain.swapChainExtent,
        });
        
        
        auto& commandBuffer = stp.buffer.commandBuffer;

        // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);
        // pushConstant.use(buffer,pipeline,PushConstantBlock(glm::vec2(1920,1080),glm::vec2(660+150,100),glm::vec2(300,100)));
        // ds.bind(device,commandBuffer,render,pipeline);
       
        // commandBuffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
        // commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint16);
        // commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0,0);
        

        // process mouse
        glm::dvec2 rawMousPos;
        glm::ivec2 realWindowSize;
        glfwGetCursorPos(stp.window, &rawMousPos.x, &rawMousPos.y);
        glfwGetFramebufferSize(stp.window, &realWindowSize.x, &realWindowSize.y);
        glm::vec2 relativeMousePosition = (rawMousPos/glm::dvec2(realWindowSize));   

        screen->draw(*this, stp, relativeMousePosition);
    }
MeshWeaver mw;
struct Chunk2{
    Buffer vertexBuffer,indexBuffer;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    void create(RenderSync* render,Device& device,CommandPool& pool,FastNoiseLite& noise,size_t xOffset,size_t yOffset,size_t zOffset){
        {
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
            
            auto t1 = std::chrono::steady_clock::now();
            mw.create(data,xOffset,yOffset,zOffset);
            auto t2 =  std::chrono::steady_clock::now();
            std::cout <<"mesh generation time:"<< std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()<<"µs" <<std::endl;
        }
        vertices = *(std::vector<Vertex>*)&mw.vertices; // me being a bad boy. Because i am lazy.
        indices = mw.index;

        if(indices.size()){
            vertexBuffer.createVertexBuffer(render,pool,vertices.data(),vertices.size());
            indexBuffer.createIndexBuffer(  render,pool,indices.data(),indices.size());
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
struct WorldRenderer{
    Image image;
    DescriptorSetLayout dsLayout;
    DescriptorSet ds;
    Camera camera;
    Pipeline pipeline;
    
    FastNoiseLite noise;
    static const size_t range = 3;
    Chunk2 chunk[range][range][range];

    void init(Game& _game,std::filesystem::path projectBaseDir,DepthBuffer& depthBuffer){

        time_t t;
        time(&t);
        noise.SetSeed(t);
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFrequency(0.005);
        //noise.SetFractalGain(0.75);
        noise.SetFractalOctaves(5);
        //noise.SetFractalLacunarity(2);    

        image.create(&_game.render,_game.commandPool,projectBaseDir/"assets"/"texture.jpg");
        
        camera.create(_game.device,&_game.render);
        dsLayout.create(_game.device,{
            DescriptorLayout(0,vk::ShaderStageFlagBits::eVertex  ,vk::DescriptorType::eUniformBuffer),
            //DescriptorLayout(1,vk::ShaderStageFlagBits::eFragment,vk::DescriptorType::eCombinedImageSampler),
        });
        ds.create(_game.device,&_game.render,dsLayout,{
            DescriptorLayout(0,vk::ShaderStageFlagBits::eVertex  ,vk::DescriptorType::eUniformBuffer),
            //Descriptor(1,vk::ShaderStageFlagBits::eFragment,image),
        });
        ds.setResource(0, camera.ubo);

        depthBuffer.create(_game.commandPool,_game.swapchain);
        pipeline.create(&_game.render,_game.device,
            projectBaseDir/"bin"/"slang.spv",
            "vertMain","fragMain",
            _game.swapchain, dsLayout,depthBuffer);

        for (size_t x = 0; x < range; x++){
            for (size_t y = 0; y < range; y++){
                for (size_t z = 0; z < range; z++){
                    chunk[x][y][z].create(&_game.render,_game.device,_game.commandPool,noise,x*32,-32+y*32,z*32);
                }
            }
            
        }
    }
    void draw(Game& _game,CommandBuffer& CB,uint32_t frameIndex,uint32_t imageIndex){
        float aspectRatio = static_cast<float>(_game.swapchain.swapChainExtent.width) / static_cast<float>(_game.swapchain.swapChainExtent.height);

        bool zoom = glfwGetKey(_game.window,GLFW_KEY_C) == GLFW_PRESS;
        camera.updateUniformBuffer(frameIndex,aspectRatio, zoom,camera.pos,camera.forward);

        auto& commandBuffer = CB.commandBuffer;
        
        {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);
            commandBuffer.setViewport(0, vk::Viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(_game.swapchain.swapChainExtent.width),
                .height = static_cast<float>(_game.swapchain.swapChainExtent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            });
            commandBuffer.setScissor(0, vk::Rect2D{
                .offset = vk::Offset2D{.x = 0,.y = 0},
                .extent = _game.swapchain.swapChainExtent,
            });
            
            //TODO: learn more about dynamic descriptors
            ds.bind(_game.device,commandBuffer,_game.render,pipeline);
    
            for (size_t x = 0; x < range; x++){
                for (size_t y = 0; y < range; y++){
                    for (size_t z = 0; z < range; z++){
                        chunk[x][y][z].draw(CB);
                    }
                }
            }
        }
    }

};
void game(Game& _game,std::filesystem::path projectBaseDir) {
    DepthBuffer depthBuffer;
    WorldRenderer wr;
    wr.init(_game, projectBaseDir,depthBuffer);
    GuiSystem* gs = nullptr;
   
    // TODO refactor the following in the future:
    auto recordCommandBuffer = [&](CommandBuffer& CB,uint32_t frameIndex,uint32_t imageIndex)
    {
        CB.begin();
        _game.swapchain.beginRendering(CB,imageIndex,&depthBuffer);
        
        Setup setup{
            .instance = _game.instance,
            .window = _game.window,
            .device = _game.device,
            .queue = _game.queue,
            .swapchain = _game.swapchain,
            .pool = _game.commandPool,
            .render = &_game.render,
            .projectBaseDir = projectBaseDir,
            .depthBuffer = depthBuffer,
            .buffer = CB
        };
        if(!gs){
            gs = new GuiSystem;
            gs->create(setup);
        }

        wr.draw(_game,CB, frameIndex, imageIndex);
        if(glfwGetKey(_game.window, GLFW_KEY_P)){
            delete gs;
            gs = 0;
        }
        if(gs)
            gs->draw(setup);
        
            
        _game.swapchain.endRendering(CB,imageIndex);
        CB.end();
    };


    


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
        fpsCounter.update();

        //camera 
        if(_game.window.grabMouse)
        {
            wr.camera.update(_game.window,fpsCounter.delta);   
        }
        
    }
    _game.device.device.waitIdle();
    delete gs;
}
int main(int argc, char const *argv[]){    
    time_t t;
    time(&t);
    srand(t);
    auto projectBaseDir = std::filesystem::canonical(argv[0]).parent_path().parent_path().parent_path();
    try{
        Game _game(projectBaseDir);
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
