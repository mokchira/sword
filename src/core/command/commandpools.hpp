#ifndef COMMAND_COMMANDPOOLS_HPP
#define COMMAND_COMMANDPOOLS_HPP

#include "rendercommands.hpp"
#include "shader.hpp"
#include "saveimage.hpp"
#include "commandtypes.hpp"
#include "watcher.hpp"

namespace sword
{

template<typename T>
using CommandPool = command::Pool<T, 5>;

//not including vocab commands
struct CommandPools
{
    CommandPool<command::LoadFragShader> loadFragShader;
    CommandPool<command::LoadVertShader> loadVertShader;
    CommandPool<command::SetSpecFloat> setSpecFloat;
    CommandPool<command::SetSpecInt> setSpecInt;
    CommandPool<command::AddAttachment> addAttachment; 
    CommandPool<command::OpenWindow> openWindow;
//    CommandPool<command::SetOffscreenDim> setOffscreenDim; not implemented yet
    CommandPool<command::CreateDescriptorSetLayout> createDescriptorSetLayout;
    CommandPool<command::CreatePipelineLayout> createPipelineLayout;
    CommandPool<command::PrepareRenderFrames> prepareRenderFrames;
    CommandPool<command::CreateGraphicsPipeline> createGraphicsPipeline;
    CommandPool<command::CreateSwapchainRenderpass> createSwapchainRenderpass;
    CommandPool<command::CreateOffscreenRenderpass> createOffscreenRenderpass;
    CommandPool<command::CreateRenderLayer> createRenderLayer;
    CommandPool<command::RecordRenderCommand> recordRenderCommand;
    CommandPool<command::CreateFrameDescriptorSets> createFrameDescriptorSets;
    CommandPool<command::AddFrameUniformBuffer> addFrameUniformBuffer;
    CommandPool<command::UpdateFrameSamplers> updateFrameSamplers;
    CommandPool<command::CompileShader> compileShader;
    CommandPool<command::CompileShaderCode> compileShaderCode;
    CommandPool<command::WatchFile> watchFile;
    CommandPool<command::SaveSwapToPng> saveSwapToPng;
    CommandPool<command::SaveAttachmentToPng> saveAttachmentToPng;
    CommandPool<command::BindUboData> bindUboData;
};

}; // namespace sword

#endif /* end of include guard: COMMAND_COMMANDPOOLS_HPP */
