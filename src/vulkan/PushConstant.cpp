#include "PushContant.hpp"

void PushConstant::create(vk::ShaderStageFlagBits shaderStages,size_t offset,size_t size){
    this->shaderStages = shaderStages;
    this->offset       = offset;

    pushConstantRange.setStageFlags(shaderStages) 
                .setOffset(offset)
                .setSize(size);
}
