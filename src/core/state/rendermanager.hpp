#ifndef STATE_RENDERMANAGER_HPP
#define STATE_RENDERMANAGER_HPP

//imp : state/rendermanager.cpp

#include <state/state.hpp>
#include <state/pipelinemanager.hpp>
#include <state/descriptormanager.hpp>
#include <state/renderpassmanager.hpp>
#include <state/shader.hpp>
#include <command/rendercommands.hpp>
#include <command/saveimage.hpp>
#include <command/commandtypes.hpp>

namespace sword
{

namespace state
{

class SaveSwapImage : public LeafState
{
public:
    const char* getName() const override { return "SaveSwapImage"; }
    void handleEvent(event::Event*) override;
    SaveSwapImage(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::SaveSwapToPng>& pool;
};

class SetRenderCommand : public LeafState
{
public:
    const char* getName() const override { return "Render"; }
    void handleEvent(event::Event*) override;
    SetRenderCommand(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    command::Pool<command::PushDraw, 1> pool;
};

class RecordRenderCommand : public LeafState
{
public:
    const char* getName() const override { return "RecordRenderCommand"; }
    void handleEvent(event::Event*) override;
    RecordRenderCommand(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::RecordRenderCommand>& pool;
};

class CreateRenderLayer : public LeafState
{
public:
    const char* getName() const override { return "CreateRenderLayer"; }
    void handleEvent(event::Event*) override;
    CreateRenderLayer(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::CreateRenderLayer>& crlPool;
};

class OpenWindow : public BriefState
{
public:
    const char* getName() const override { return "OpenWindow"; }
    OpenWindow(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::OpenWindow>& owPool;
};

class PrepareRenderFrames : public BriefState
{
public:
    const char* getName() const override { return "PrepareRenderFrames"; }
    PrepareRenderFrames(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::PrepareRenderFrames>& prfPool;
};

class RenderManager final : public BranchState
{
public:
    const char* getName() const override { return "render_manager"; }
    void handleEvent(event::Event*) override;
    RenderManager(StateArgs, Callbacks cb);
private:
    enum class Op : Option {saveSwapImage, printReports, render, openWindow, prepRenderFrames, shaderManager, 
        descriptorManager, renderPassManager, pipelineManager, createRenderLayer, recordRenderCommand};

    PipelineManager pipelineManager;
    RenderPassManager rpassManager;
    DescriptorManager descriptorManager;
    ShaderManager shaderManager;
    OpenWindow openWindow;
    PrepareRenderFrames prepRenderFrames;
    CreateRenderLayer createRenderLayer;
    RecordRenderCommand recordRenderCommand;
    SaveSwapImage saveSwapImage;
    SetRenderCommand setRenderCommand;

    Reports<RenderLayerReport> renderLayersReports;
    Reports<RenderCommandReport> renderCommandReports;

    CommandPool<command::RecordRenderCommand>& rrcPool;

    command::Pool<command::PushDraw, 1> pool;

    bool createdDescSetLayout{false};

    void setRenderCommandF(int cmdIndex);
    void onPrepRenderFrames();
    void onOpenWindow();
    void receiveDescriptorSetLayoutReport(const DescriptorSetLayoutReport*);
    void printReports() const;
    void rerecordRenderCommand(RenderCommandReport*);
    void receiveGraphicsPipelineReport(const GraphicsPipelineReport*);
};

} // namespace state

} // namespace sword

#endif /* end of include guard: STATE_RENDERMANAGER_HPP */
