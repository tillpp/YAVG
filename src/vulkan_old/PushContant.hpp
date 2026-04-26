#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/CommandBuffer.hpp"
#include "vulkan/Pipeline.hpp"

struct PushConstantBlock{
    glm::vec2 position;
    glm::vec2 size;
    PushConstantBlock(glm::vec2 screenSize,glm::vec2 position,glm::vec2 size){
        this->position = position/screenSize;
        this->size     = size    /screenSize;
    }
};
class PushConstant{
public:
    vk::PushConstantRange pushConstantRange;
    void create(){
        // Set up push constant range for material properties
        pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex) // Which shader stages can access the push constants
                    .setOffset(0)
                    .setSize(sizeof(PushConstantBlock)); // Size of our push constant data
    }
    void use(CommandBuffer& cb,Pipeline& pipeline,PushConstantBlock pushConstants){
        // Push constants to shader using vk::raii
        cb.commandBuffer.pushConstants<PushConstantBlock>(
            *pipeline.pipelineLayout,
            vk::ShaderStageFlagBits::eVertex, // Which shader stages will receive the data
            0, // Offset
            {pushConstants} // Data
        );  
    }
};