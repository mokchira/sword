#include <state/descriptormanager.hpp>

namespace sword
{

namespace state
{


CreateFrameDescriptorSets::CreateFrameDescriptorSets(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, cfdsPool{sa.cp.createFrameDescriptorSets} 
{
    sa.rg.createFrameDescriptorSets = this;
}

void CreateFrameDescriptorSets::onEnterExt()
{
    std::cout << "Enter the descriptor set layout names" << std::endl;
}

void CreateFrameDescriptorSets::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto stream = toCommandLine(event)->getStream();
        std::vector<std::string> layoutNames;
        std::string name;
        while (stream >> name)
            layoutNames.push_back(name);
        auto cmd = cfdsPool.request(reportCallback(), layoutNames);
        pushCmd(std::move(cmd));
        event->setHandled();
        popSelf();
    }
}

InitFrameUbos::InitFrameUbos(StateArgs sa) :
    LeafState{sa, {}}, pool{sa.cp.addFrameUniformBuffer}
{}

void InitFrameUbos::onEnterExt()
{
    std::cout << "Enter a binding number" << std::endl;
}

void InitFrameUbos::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto size = ce->getArg<size_t, 0>();
        auto binding = ce->getArg<int, 1>();
        auto cmd = pool.request(size, binding);
        pushCmd(std::move(cmd));
        event->setHandled();
        popSelf();
    }
}

UpdateFrameSamplers::UpdateFrameSamplers(StateArgs sa) :
    LeafState{sa, {}}, pool{sa.cp.updateFrameSamplers}
{}

void UpdateFrameSamplers::onEnterExt()
{
    std::cout << "Enter a binding number" << std::endl;
}

void UpdateFrameSamplers::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
//        auto ce = toCommandLine(event);
//        auto binding = ce->getFirst<int>();
//        auto cmd = pool.request(binding);
//        pushCmd(std::move(cmd));
//        Need to disable this until we figure out how to pass it a vector of image addresses
        event->setHandled();
        popSelf();
    }
}

DescriptorManager::DescriptorManager(StateArgs sa, Callbacks cb, ReportCallbackFn<DescriptorSetLayoutReport> dslrCb) :
    BranchState{sa, cb, 
        {
            {"create_frame_descriptor_sets", opcast(Op::createFrameDescriptorSets)},
            {"print_reports", opcast(Op::printReports)},
            {"descriptor_set_layout_manager", opcast(Op::descriptorSetLayoutMgr)},
            {"init_frame_ubos", opcast(Op::initFrameUBOs)},
            {"update_frame_samplers", opcast(Op::updateFrameSamplers)}
        }
    },
    createFrameDescriptorSets{sa, {
        [this](){ activate(opcast(Op::initFrameUBOs));},
        [this](Report* r){ addReport(r, &descriptorSetReports); }}},
    descriptorSetLayoutMgr{sa, {
        [this](){ activate(opcast(Op::descriptorSetLayoutMgr));}},
        [this](const DescriptorSetLayoutReport* r){ receiveDescriptorSetLayoutReport(r); }
    },
    initFrameUbos{sa},
    updateFrameSamplers{sa},
    receivedDSLReportCb{dslrCb}
{
    activate(opcast(Op::printReports));
    activate(opcast(Op::descriptorSetLayoutMgr));
}

void DescriptorManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto cmdEvent = toCommandLine(event);
        auto option = extractCommand(cmdEvent);
        if (!option) return;
        switch (opcast<Op>(*option))
        {
            case Op::createFrameDescriptorSets: pushState(&createFrameDescriptorSets); deactivate(opcast(Op::createFrameDescriptorSets)); break;
            case Op::printReports: printReports(); break;
            case Op::descriptorSetLayoutMgr: pushState(&descriptorSetLayoutMgr); deactivate(opcast(Op::descriptorSetLayoutMgr)); break;
            case Op::initFrameUBOs: pushState(&initFrameUbos); break;
            case Op::updateFrameSamplers: pushState(&updateFrameSamplers); break;
        }
    }
}

void DescriptorManager::printReports()
{
    for (const auto& report : descriptorSetReports) 
    {
        std::invoke(*report);
    }
}

void DescriptorManager::activateDSetLayoutNeeding()
{
    if (descriptorSetLayoutMgr.hasCreatedLayout())
        activate(opcast(Op::createFrameDescriptorSets));
}

void DescriptorManager::receiveDescriptorSetLayoutReport(const DescriptorSetLayoutReport* r)
{
    descriptorSetLayoutReports.push_back(r);
    createFrameDescriptorSets.addToVocab(r->getObjectName());
    if (descriptorSetLayoutReports.size() == 1) //pushed first one
        activate(opcast(Op::createFrameDescriptorSets));
    if (receivedDSLReportCb)
        std::invoke(receivedDSLReportCb, r);
}

}; // namespace state

}; // namespace sword
