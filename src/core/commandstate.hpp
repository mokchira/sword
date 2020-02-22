#include "../core/event.hpp"
#include "../core/renderer.hpp"
#include <stack>
#include <map>

class Application;
class AddAttachment;

#define STATE_BASE(name) \
    void handleEvent(Event* event, EditStack*, CommandStack*) override;\
    void onEnter(Application* app) override;\
    static const char* getName() {return name;}

#define CMD_BASE(name) \
    void execute(Application*) override;\
    static const char* getName() {return name;}

template <typename T>
class OptionMap
{
public:
    using Element = std::pair<std::string, T>;
    OptionMap(
            std::initializer_list<Element> avail,
            std::initializer_list<Element> reserved) : 
        optionsAvailable(avail),
        optionsReserved(reserved) {}
    inline T findOption(const std::string& s) const 
    {
        T t;
        for (const auto& item : optionsAvailable) 
            if (item.first == s)
                t = item.second;
        return t; //should be 0 if not found 
    }
    inline void remove(T t) 
    {
        const size_t size = optionsAvailable.size();
        for (int i = 0; i < size; i++) 
           if (optionsAvailable[i].second == t)
              optionsAvailable.erase(optionsAvailable.begin() + i); 
    }
    inline std::vector<std::string> getStrings() const
    {
        std::vector<std::string> vec;
        vec.reserve(optionsAvailable.size());
        for (const auto& item : optionsAvailable) 
            vec.push_back(item.first);
        return vec;
    }
    inline void add(T t)
    {
        const size_t size = optionsReserved.size();
        for (int i = 0; i < size; i++) 
        {
            if (optionsReserved[i].second == t)
            {
                Element element = optionsReserved[i];
                optionsAvailable.push_back(element);
                optionsReserved.erase(optionsReserved.begin() + i);
            }   
        }
    }
private:
    std::vector<std::pair<std::string, T>> optionsAvailable;
    std::vector<std::pair<std::string, T>> optionsReserved;
};

struct ShaderReport
{
    const std::string name{"unitilialized"};
    const char* type{"uninilialized"};
    int specint0{0};
    int specint1{0};
    float specfloat0{0};
    float specfloat1{0};
    ShaderReport(std::string n, const char* t, int i0, int i1, float f0, float f1) :
        name{n}, type{t}, specint0{i0}, specint1{i1}, specfloat0{f0}, specfloat1{f1} {}
    void operator()() const 
    {
        std::cout << "================== Shader Report ==================" << std::endl;
        std::cout << "Name:                  " << name << std::endl;
        std::cout << "Type:                  " << type << std::endl;
        std::cout << "Spec Constant Int 0:   " << specint0 << std::endl;
        std::cout << "Spec Constant Int 1:   " << specint1 << std::endl;
        std::cout << "Spec Constant Float 0: " << specfloat0 << std::endl;
        std::cout << "Spec Constant Float 1: " << specfloat1 << std::endl;
    }
    
};

struct GraphicsPipelineReport
{
    std::string name{"default"};
    std::string pipelineLayout{"default"};
    std::string vertshader{"default"};
    std::string fragshader{"default"};
    std::string renderpass{"default"};
    int regionX;
    int regionY;
    uint32_t areaX;
    uint32_t areaY;
    bool is3d{false};
    GraphicsPipelineReport(
            std::string parm1,
            std::string parm2,
            std::string parm3,
            std::string parm4,
            std::string parm5,
            int i1,
            int i2,
            uint32_t ui1,
            uint32_t ui2,
            bool is3d) :
        name{parm1}, pipelineLayout{parm2}, vertshader{parm3}, fragshader{parm4},
        renderpass{parm5}, regionX{i1}, regionY{i2}, areaX{ui1}, areaY{ui2}, is3d{is3d} {}
    void operator()() const
    {
        std::cout << "================== Graphics Pipeline Report ==================" << std::endl;
        std::cout << "Name:            " << name << std::endl;
        std::cout << "Pipeline layout: " << pipelineLayout << std::endl;
        std::cout << "Vert Shader:     " << vertshader << std::endl;
        std::cout << "Frag Shader:     " << fragshader << std::endl;
        std::cout << "Renderpass:      " << renderpass << std::endl;
        std::cout << "Coords:         (" << regionX << ", " << regionY << ")" << std::endl;
        std::cout << "Area:           (" << areaX << ", " << areaY << ")" << std::endl;
        std::cout << "Is it 3d?        " << is3d << std::endl;
    }
};

struct RenderpassReport
{
    const std::string name{"unitilialized"};
    RenderpassReport(std::string n) :
        name{n} {}
    void operator()() const 
    {
        std::cout << "================== Shader Report ==================" << std::endl;
        std::cout << "Name:                  " << name << std::endl;
    }
    
};

template <class T>
class Stack
{
public:
    friend class Application;
    inline void push(T&& item) {items.push_back(std::forward<T>(item));}
    inline void pop() {items.pop_back();}
    inline T top() const {return items.back();}
    inline bool empty() const {return items.empty();}
protected:
    std::vector<T> items;
};

template <class T>
class ForwardStack : public Stack<T>
{
public:
    inline auto begin() {return this->items.begin();}
    inline auto end() {return this->items.end();}
};

template <class T>
class ReverseStack : public Stack<T>
{
public:
    inline auto begin() {return this->items.rbegin();}
    inline auto end() {return this->items.rend();}
};

struct FragmentInput
{
    float time;
	float mouseX{0};
	float mouseY{0};	
    float r{1.};
    float g{1.};
    float b{1.};
    float a{1.};
    float brushSize{1.};
    glm::mat4 xform{1.};
};

namespace Command
{

class Command 
{
public:
    virtual ~Command() = default;
    virtual void execute(Application*) = 0;
    static const char* getName() {return "commandBase";}
    inline bool isAvailable() const {return !inUse;}
    template <typename... Args> inline void set(Args... args) {inUse = true;}
    void reset() {inUse = false;}
protected:
    Command() = default;
    bool inUse{false};
};

class LoadFragShader: public Command
{
public:
    CMD_BASE("loadFragShader");
    inline void set(std::string name) {Command::set(); shaderName = name;}
private:
    std::string shaderName;
};

class LoadVertShader: public Command
{
public:
    CMD_BASE("loadVertShader");
    inline void set(std::string name) {Command::set(); shaderName = name;}
private:
    std::string shaderName;
};

class SetSpecFloat: public Command
{
public:
    CMD_BASE("setSpecFloat");
    inline void set(std::string name, std::string t, float first, float second) {
        Command::set(); shaderName = name; type = t; x = first; y = second;}
private:
    std::string shaderName;
    std::string type;
    float x{0};
    float y{0};
};

class SetSpecInt: public Command
{
public:
    CMD_BASE("setSpecInt");
    inline void set(std::string name, std::string t, int first, int second) {
        Command::set(); shaderName = name; type = t; x = first; y = second;}
private:
    std::string shaderName;
    std::string type;
    int x{0};
    int y{0};
};

class AddAttachment: public Command
{
public:
    CMD_BASE("addAttachment");
    inline void set(std::string name, int x, int y, bool sample, bool transfer)
    {
        assert(x > 0 && y > 0 && "Bad attachment size");
        Command::set();
        attachmentName = name;
        isSampled = sample;
        isTransferSrc = transfer;
        dimensions.setWidth(x);
        dimensions.setHeight(y);
    }
private:
    std::string attachmentName;
    vk::Extent2D dimensions{{500, 500}};
    bool isSampled{false};   
    bool isTransferSrc{false};
};

class OpenWindow : public Command
{
public:
    CMD_BASE("openWindow")
};

class SetOffscreenDim : public Command
{
public:
    CMD_BASE("setOffscreenDim");
private:
    uint16_t w{0}, h{0};
};

class CreatePipelineLayout : public Command
{
public:
    CreatePipelineLayout();
    CMD_BASE("createPipelineLayout")
private:
    std::string layoutname{"foo"};
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

class PrepareRenderFrames : public Command
{
public:
    CMD_BASE("prepareRenderFrames");
    inline void set(XWindow* window) {Command::set(); this->window = window;}
private:
    XWindow* window{nullptr};
};

class CreateGraphicsPipeline : public Command
{
public:
    CMD_BASE("createGraphicsPipeline");
    inline void set(
            std::string parm1,
            std::string parm2,
            std::string parm3,
            std::string parm4,
            std::string parm5,
            vk::Rect2D renderAreaSet,
            bool has3dGeo) 
    {
        name = parm1;
        pipelineLayout = parm2;
        vertshader = parm3;
        fragshader = parm4;
        renderpass = parm5;
        renderArea = renderAreaSet;
        is3d = has3dGeo;
    }
private:
    std::string name{"default"};
    std::string pipelineLayout{"default"};
    std::string vertshader{"default"};
    std::string fragshader{"default"};
    std::string renderpass{"default"};
    vk::Rect2D renderArea{{0, 0}, {100, 100}};
    bool is3d{false};
};

class CreateSwapchainRenderpass : public Command
{
public:
    CMD_BASE("create_swapchain_render_pass");
    inline void set(std::string name) {Command::set(); rpassName = name;}
private:
    std::string rpassName;
};

template <typename T>
class Pool
{
public:
    template <typename P> using Pointer = std::unique_ptr<P, std::function<void(P*)>>;

    Pool(size_t size) : size{size}, pool(size) {}

    template <typename... Args> Pointer<Command> request(Args... args)
    {
        for (int i = 0; i < size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].set(args...);
                Pointer<Command> ptr{&pool[i], [](Command* t)
                    {
                        t->reset();
                    }};
                return ptr; //tentatively may need to be std::move? copy should be ellided tho
            }    
        return nullptr;
    }

    void printAll() const 
    {
        for (auto& i : pool) 
            i.print();
    }

    static const char* getName() {return T::getName();}

private:

    template <typename... Args> void initialize(Args&&... args)
    {
        for (int i = 0; i < size; i++) 
            pool.emplace_back(args...);   
    }

    const size_t size;
    std::vector<T> pool;
};

};

typedef ForwardStack<std::unique_ptr<Command::Command, std::function<void(Command::Command*)>>> CommandStack;

class EditStack;

namespace State
{

class State;
typedef ReverseStack<State*> StateStack;

typedef std::vector<std::string> Vocab;

class State
{
public:
    virtual void handleEvent(Event* event, EditStack*, CommandStack*) = 0;
    virtual void onEnter(Application*) = 0;
};

class ShaderManager: public State
{
public:
    STATE_BASE("shadermanager");
private:
    Command::Pool<Command::LoadFragShader> loadFragPool{10};
    Command::Pool<Command::LoadVertShader> loadVertPool{10};
    Command::Pool<Command::SetSpecFloat> ssfPool{10};
    Command::Pool<Command::SetSpecInt> ssiPool{10};
    enum class Option {null, loadFragShaders, loadVertShaders, shaderReport, setSpecInts, setSpecFloats};
    using OptionMap = std::map<std::string, Option>;
    OptionMap opMap{
        {"loadfragshaders", Option::loadFragShaders},
        {"loadvertshaders", Option::loadVertShaders},
        {"report", Option::shaderReport},
        {"setspecints", Option::setSpecInts},
        {"setspecfloats", Option::setSpecFloats},
        {"setwindowresolution", Option::setSpecFloats}};
    Option mode{Option::null};
    std::map<std::string, ShaderReport> shaderReports;
    std::vector<std::string> shaderNames;
};

class AddAttachment: public State
{
public:
    STATE_BASE("addAttachment");
private:
    Command::Pool<Command::AddAttachment> addAttPool{20};
    int width, height;
    enum class Option {null, setdimensions, fulldescription, addsamplesources};
    using Vocabulary = std::map<std::string, Option>;
    Vocabulary vocab{
        {"setDimensions", Option::setdimensions},
        {"fulldescription", Option::fulldescription},
        {"addSampleSources", Option::addsamplesources}};
    Option mode{Option::null};
};

class Paint : public State
{
public:
    STATE_BASE("paint")
private:
};

class RenderpassManager: public State
{
public:
    STATE_BASE("renderpassManager");
private:    
    Command::Pool<Command::CreateSwapchainRenderpass> csrPool{1};
    enum class Option {null, createSwapRenderpass};
    using OptionMap = std::map<std::string, Option>;
    OptionMap opMap{
        {"create_swap_renderpass", Option::createSwapRenderpass}};
    std::map<std::string, RenderpassReport> shaderReports;
    Option mode{Option::null};
};

class PipelineManager: public State
{
public:
    STATE_BASE("pipelineManager");
private:
    Command::Pool<Command::CreateGraphicsPipeline> cgpPool{10};
    enum class Option : uint8_t {null, createGraphicsPipeline, report};
    std::map<std::string, Option> opMap{
        {"createGraphicsPipeline", Option::createGraphicsPipeline},
        {"report", Option::report }};
    Option mode{Option::null};
    std::map<std::string, GraphicsPipelineReport> gpReports;
};

class InitRenderer : public State
{
public:
    inline InitRenderer(XWindow& the_window) : window{the_window} {}
    STATE_BASE("initRenderer");
private:
    Command::Pool<Command::PrepareRenderFrames> prfPool{1};
    Command::Pool<Command::CreatePipelineLayout> cplPool{1};
    Command::Pool<Command::OpenWindow> owPool{1};
    RenderpassManager rpassManager;
    PipelineManager pipelineManager;
    ShaderManager shaderManager;
    AddAttachment addAttachState;
    enum class Option {null, shaderManager, openWindow, addattachState, prepRenderFrames, createPipelineLayout, rpassManager, pipelineManager, all};
    OptionMap<Option> opMap{
        {
            {"createPipelineLayout", Option::createPipelineLayout},
            {"openWindow", Option::openWindow},
            {"addAttachState", Option::addattachState},
            {"shaderManager", Option::shaderManager},
            {"all", Option::all}
        },{
            {"rpassManager", Option::rpassManager},
            {"prepRenderFrames", Option::prepRenderFrames},
            {"pipelineManager", Option::pipelineManager}
        }};
          
    XWindow& window;
    bool preparedFrames{false};
    bool createdLayout{false};
};

class Director: public State
{
public:
    STATE_BASE("director");
    Director(XWindow& window) : initRenState{window} {}
    template <typename T> inline void pushStateFn(std::string& str, T& t, StateStack* stack) const
    {
        if (str == t.getName())
            stack->push(&t);
    }
private:
    InitRenderer initRenState;
    enum class Option {null, initRenState};
    using OptionMap = std::map<std::string, Option>;
    OptionMap opMap{
        {"initRenderer", Option::initRenState}};
};

};

typedef ReverseStack<State::State*> StateStack;

class EditStack : public ReverseStack<State::State*>
{
public:
    inline void reload(State::State* ptr) {push(std::forward<State::State*>(ptr)); push(nullptr);}
};

class Application
{
public:
    Application(uint16_t w, uint16_t h, bool recordevents, bool readevents);
    void setVocabulary(State::Vocab);
    void run();
    void popState();
    void pushState(State::State* const);
    void createPipelineLayout();
    void loadDefaultShaders();
    void createDefaultRenderPasses();
    void initUBO();

    void readEvents(std::ifstream&);

    Context context;
    XWindow window;
    Renderer renderer;
    EventHandler ev;
    vk::Extent2D offscreenDim;
    vk::Extent2D swapDim;;

private:
    const bool recordevents;
    const bool readevents;
    
    StateStack stateStack;
    EditStack stateEdits;
    CommandStack cmdStack;
    FragmentInput fragInput;

    State::Director dirState;
};
