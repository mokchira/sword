#ifndef STATE_STATE_HPP
#define STATE_STATE_HPP

#include <command/commandtypes.hpp>
#include <command/commandpools.hpp>
#include <state/report.hpp>
#include <state/editstack.hpp>
#include <types/stack.hpp>
#include <types/map.hpp>
#include <event/event.hpp>
#include <command/vocab.hpp>
#include "option.hpp"
#include "vocab.hpp"

#define STATE_BASE(name) \
    void handleEvent(Event* event) override;\
    virtual const char* getName() const override {return name;}

namespace sword { namespace state { class State; }; };

namespace sword
{

namespace render{ class Context;}
    
namespace state
{

template <typename R>
using ReportCallbackFn = std::function<void(const R*)>;
template <typename R>
using ReportCallbacks = std::vector<ReportCallbackFn<R>>;
using OwningReportCallbackFn = std::function<void(Report*)>;
using ExitCallbackFn = std::function<void()>;
using GenericCallbackFn = std::function<void()>;

template <typename T>
using Reports = std::vector<std::unique_ptr<T>>;

//constexpr bool isCommandLine(event::Event* event) { return event->getCategory() == event::Category::CommandLine;}
constexpr event::CommandLine* toCommandLine(event::Event* event) { return static_cast<event::CommandLine*>(event);}
constexpr event::Window* toWindowEvent(event::Event* event) { return static_cast<event::Window*>(event);}
constexpr event::MousePress* toMousePress(event::Window* event) { return static_cast<event::MousePress*>(event);}

enum class StateType : uint8_t {leaf, branch, brief};

//forward decs
class LoadFragShaders;
class LoadVertShaders;
class SetSpec;
class OpenWindow;
class PrepareRenderFrames;
class CreateFrameDescriptorSets;
class CreateDescriptorSetLayout;
class CreateRenderPass;
class CreatePipelineLayout;
class CreateGraphicsPipeline;
class CreateRenderLayer;
class CompileShader;
class RecordRenderCommand;

struct Register
{
    const LoadFragShaders* loadFragShaders;
    const LoadVertShaders* loadVertShaders;
    const SetSpec* setSpec;
    const OpenWindow* openWindow;
    const PrepareRenderFrames* prepareRenderFrames;
    const CreateFrameDescriptorSets* createFrameDescriptorSets;
    const CreateDescriptorSetLayout* createDescriptorSetLayout;
    const CreateRenderPass* createRenderPass;
    const CreatePipelineLayout* createPipelineLayout;
    const CreateGraphicsPipeline* createGraphicsPipeline;
    const CreateRenderLayer* createRenderLayer;
    const CompileShader* compileShader;
    const RecordRenderCommand* recordRenderCommand;
};

struct StateArgs
{
    EditStack& es;
    command::Queue& cs;
    CommandPools& cp;
    Register& rg;
    render::Context& ct;
};

struct Callbacks
{
    ExitCallbackFn ex{nullptr};
    OwningReportCallbackFn rp{nullptr};
    GenericCallbackFn gn{nullptr};
};


class OptionMap
{
public:
    using Element = std::pair<std::string, Option>;
    OptionMap(std::initializer_list<Element> init) :
        map{init} 
    {
        std::sort(map.begin(), map.end(), [](const Element& e1, const Element& e2) { return e1.second < e2.second; });
    }

    std::vector<std::string> getStrings()
    {
        return map.getKeys();
    }

    std::optional<Option> findOption(const std::string& s, OptionMask mask) const
    {
        return map.findValue(s, mask);
    }
private:
    SmallMap<std::string, Option> map;
};

class State
{
public:
    virtual void handleEvent(event::Event* event) = 0;
    virtual const char* getName() const = 0;
    virtual StateType getType() const = 0;
    virtual void beginFrame() {}
    virtual void endFrame() {}
    virtual ~State() = default;
    void onEnter();
    void onExit();
    std::vector<std::string> getVocab();
//    virtual std::vector<const Report*> getReports() const {return {};};
protected:
    State(command::Queue& cs) : cmdStack{cs} {}
    State(command::Queue& cs, ExitCallbackFn callback) : cmdStack{cs}, onExitCallback{callback} {}
    void setVocab(std::vector<std::string> strings);
    void clearVocab() { vocab.clear(); }
    void addToVocab(std::string word) { vocab.push_back(word); }
    void updateVocab();
    void printVocab();
    void pushCmd(command::Vessel);
    void setVocabMask(OptionMask* mask) { vocab.setMaskPtr(mask); }
private:
    command::Pool<command::UpdateVocab, 3> uvPool;
    command::Pool<command::PopVocab, 3> pvPool;
    command::Pool<command::AddVocab, 3> avPool;
    Vocab vocab;
    command::Queue& cmdStack;
    ExitCallbackFn onExitCallback{nullptr};
    virtual void onEnterImp();
    virtual void onExitImp();
    virtual void onEnterExt() {}
    virtual void onExitExt() {}
};

//TODO: need to delete copy assignment and copy constructors
class LeafState : public State
{
public:
    StateType getType() const override { return StateType::leaf; }
    void addToVocab(std::string word) { State::addToVocab(word); }
    const OwningReportCallbackFn reportCallback() const { return reportCallbackFn; }
protected:
    LeafState(StateArgs sa, Callbacks cb) :
        State{sa.cs, cb.ex}, cmdStack{sa.cs}, editStack{sa.es}, reportCallbackFn{cb.rp} {}
    void popSelf() { editStack.popState(); }
    void pushCmd(command::Vessel ptr);

    template<typename R>
    R* findReport(const std::string& name, const std::vector<std::unique_ptr<R>>& reports)
    {
        for (const auto& r : reports) 
        {
            if (r->getObjectName() == name)
                return r.get();
        }
        return nullptr;
    }

private:
    command::Pool<command::SetVocab, 3> svPool;
    EditStack& editStack;
    OwningReportCallbackFn reportCallbackFn{nullptr};
    command::Queue& cmdStack;

    void onExitImp() override final;
    void onEnterImp() override final;
    virtual void onEnterExt() override {}
    virtual void onExitExt() override {}

};

class BriefState : public LeafState
{
public:
    StateType getType() const override { return StateType::brief; }
    void handleEvent(event::Event*) override final {}; //no events for this guy
protected:
    BriefState(StateArgs sa, Callbacks cb) :
        LeafState{sa, cb} 
    {}
};

//TODO: make the exit callback a requirement for branch states
//      and make Director a special state that does not require it (or just make it null)
//TODO: need to delete copy assignment and copy constructors
class BranchState : public State
{
friend class Director;
public:
    ~BranchState() = default;
    StateType getType() const override final { return StateType::branch; }
    template<typename R>
    static void addReport(Report* ptr, std::vector<std::unique_ptr<R>>* derivedReports, ReportCallbackFn<R> callback = {})
    {
        if (!ptr)
        {
            std::cout << "BranchState::addReport: null ptr passed, returning immediately." << '\n';
            return;
        }
        auto derivedPtr = dynamic_cast<R*>(ptr); 
        if (derivedPtr)
        {
            derivedReports->emplace_back(derivedPtr);
            if (callback)
            {
                std::invoke(callback, derivedPtr);
            }
        }
        else
        {
            std::cout << "Bad report pointer passed." << std::endl;
            delete ptr;
        }
    }

protected:
    using Element = std::pair<std::string, Option>;
    BranchState(StateArgs sa, Callbacks cb, std::initializer_list<Element> ops) :
        State{sa.cs, cb.ex}, editStack{sa.es}, options{ops}
    {
        setVocab(options.getStrings());
        setVocabMask(&topMask); 
    }
    void pushState(State* state);
    Optional extractCommand(event::Event*);
    void activate(Option op) { topMask.set(op); }
    void deactivate(Option op) { topMask.reset(op); }
    void updateActiveVocab();
    void printMask();

private:
    OptionMask topMask;
    std::vector<const Report*> reports;
    OptionMap options;
    EditStack& editStack;
};



}; //state

}; //sword

#endif /* end of include guard: STATE_STATE_HPP */
