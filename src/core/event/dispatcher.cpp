#include "dispatcher.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <cassert>
#include <state/vocab.hpp>
#include <thread>
#include <util/outformat.hpp>
#include <util/debug.hpp>

namespace sword
{

namespace event
{

typedef void (EventDispatcher::*pEventFunc)(xcb_generic_event_t* event);

constexpr uint32_t RL_DELAY = 100;
float normCoords(int16_t windowCoord, int16_t extent)
{
	float x = (float)windowCoord / extent; // 0<x<1
	x = x * 2 - 1;
	return x;
}

int abortHelper(int count, int key)
{
    rl_replace_line("q", 0);
    rl_done = 1;
    return 0;
}

EventDispatcher::EventDispatcher(const render::Window& window):
	window{window}, fileWatcher{eventQueue}
{
    std::cout << "event dispatcher ctor called" << std::endl;
    rl_attempted_completion_function = completer;
    rl_bind_key(27, abortHelper);
}

EventDispatcher::~EventDispatcher()
{
    keepWindowThread = false;
    keepCommandThread = false;
}

char* EventDispatcher::completion_generator(const char* text, int state)
{
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0)
    {
        matches.clear();
        match_index = 0;
    
        std::string textstr(text);
        for (auto word : vocabulary)
        {
            if (word.size() >= textstr.size() && word.compare(0, textstr.size(), textstr) == 0)
            {
                matches.push_back(word);
            }
        }
    }
    
    if (match_index >= matches.size())
    {
        return nullptr;
    }
    else
    {
        return strdup(matches[match_index++].c_str());
    }
}

char** EventDispatcher::completer(const char* text, int start, int end)
{
    rl_attempted_completion_over = 1;

    return rl_completion_matches(text, EventDispatcher::completion_generator);
}

void EventDispatcher::setVocabulary(std::vector<std::string> vocab)
{
    vocabulary = vocab;
}

void EventDispatcher::updateVocab()
{
    vocabulary.clear();
    for (const auto& vptr : vocabPtrs) 
    {
        auto words = vptr->getWords();
        for (const auto& word : words) 
            vocabulary.push_back(word);   
    }
}

void EventDispatcher::addVocab(const state::Vocab* vptr)
{
    vocabPtrs.push_back(vptr);
    updateVocab();
}

void EventDispatcher::popVocab()
{
    vocabPtrs.pop_back();
    updateVocab();
}

void EventDispatcher::fetchCommandLineInput()
{
    std::string input;

    char* buf = readline(">> ");

    if (buf && *buf) //dont add empty lines to history
        add_history(buf);

    input = std::string(buf);

    free(buf);

    if (input == "quit")
    {
        fileWatcher.stop();
        keepCommandThread = false;   
    }

    std::stringstream ss{input};
    std::string catcher;

    while (ss >> catcher)
    {
        if (catcher == "q")
        {
            Vessel event = aPool.request();
            eventQueue.push(std::move(event));
            std::cout << "Aborting operation" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(RL_DELAY));
            return;
        }
    }

    auto event = clPool.request(input);
    lock.lock();
    eventQueue.push(std::move(event));
    lock.unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(RL_DELAY));
}

void EventDispatcher::fetchWindowInput()
{	
    auto* event = window.waitForEvent();
    assert (event);
    Vessel curEvent;
	switch (static_cast<WindowEventType>(event->response_type))
	{
        case WindowEventType::Motion: 
		{
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
            curEvent = mmPool.request(motion->event_x, motion->event_y);
            break;
		}
        case WindowEventType::MousePress:
		{
            xcb_button_press_event_t* press = 
                (xcb_button_press_event_t*)event;
            curEvent = mpPool.request(press->event_x, press->event_y, static_cast<symbol::MouseButton>(press->detail));
            break;
		}
        case WindowEventType::MouseRelease:
		{
            xcb_button_press_event_t* press = 
                (xcb_button_press_event_t*)event;
            curEvent = mrPool.request(press->event_x, press->event_y, static_cast<symbol::MouseButton>(press->detail));
            break;
		}
        case WindowEventType::Keypress:
		{
			xcb_key_press_event_t* keyPress =
				(xcb_key_press_event_t*)event;
            //will cause this to be the last iteration of the window loop
            if (static_cast<symbol::Key>(keyPress->detail) == symbol::Key::Esc)
                keepWindowThread = false;
            curEvent = kpPool.request(keyPress->event_x, keyPress->event_y, static_cast<symbol::Key>(keyPress->detail));
            break;
		}
        case WindowEventType::Keyrelease:
        {
            xcb_key_release_event_t* keyRelease = 
                (xcb_key_release_event_t*)event;
            curEvent = krPool.request(keyRelease->event_x, keyRelease->event_y, static_cast<symbol::Key>(keyRelease->detail));
            break;
        }
        case WindowEventType::EnterWindow:
        {
            std::cout << "Entered window!" << std::endl;
            break;
        }
        case WindowEventType::LeaveWindow:
        {
            xcb_leave_notify_event_t* leaveEvent = (xcb_leave_notify_event_t*)event;
            curEvent = lwPool.request();
            break;
        }
	}
	free(event);
    eventQueue.push(std::move(curEvent));
}

void EventDispatcher::runCommandLineLoop()
{
    SWD_THREAD_MSG("Commandline thread started.");
    while (keepCommandThread)
    {
        fetchCommandLineInput();
    }
    SWD_THREAD_MSG("Commandline thread exitted.");
}

void EventDispatcher::runWindowInputLoop()
{
    SWD_THREAD_MSG("Window thread started.");
    while(keepWindowThread)
    {
        fetchWindowInput();
    }
    SWD_THREAD_MSG("Window thread exitted.");
}

void EventDispatcher::pollEvents()
{
    std::thread t0(&EventDispatcher::runCommandLineLoop, this);
    std::thread t1(&EventDispatcher::runWindowInputLoop, this);
    std::thread t2(&FileWatcher::run, &fileWatcher);
    t0.detach();
    t1.detach();
    t2.detach();
}

static int clCount = 0;

void EventDispatcher::readEvents(std::ifstream& is, int eventPops)
{
    bool reading{true};
    while (is.peek() != EOF)
    {
        Category ec = Event::unserializeCategory(is);
        if (ec == Category::CommandLine)
        {
            auto event = clPool.request("foo");
            auto cmdevent = static_cast<CommandLine*>(event.get());
            cmdevent->unserialize(is);
            eventQueue.push(std::move(event));
            clCount++;
            std::cout << "pushed commandline event" << std::endl;
            std::cout << "Cl count: " << clCount << std::endl;
            std::cout << "Cl content: " << cmdevent->getInput() << '\n';
        }
        if (ec == Category::Abort)
        {
            auto abrt = aPool.request();
            eventQueue.push(std::move(abrt));
            std::cout << "pushed abort event" << std::endl;
        }
    }
    for (int i = 0; i < eventPops; i++) 
    {
        eventQueue.pop();
        std::cout << "popped from queue" << std::endl;
    }
}

void EventDispatcher::readEvent(std::ifstream& is)
{
    if (is.peek() != EOF)
    {
        Category ec = Event::unserializeCategory(is);
        if (ec == Category::CommandLine)
        {
            auto event = clPool.request("foo");
            auto cmdevent = static_cast<CommandLine*>(event.get());
            cmdevent->unserialize(is);
            eventQueue.push(std::move(event));
            clCount++;
            std::cout << "pushed commandline event" << std::endl;
            std::cout << "Cl count: " << clCount << std::endl;
            std::cout << "Cl content: " << cmdevent->getInput() << '\n';
        }
        if (ec == Category::Abort)
        {
            auto abrt = aPool.request();
            eventQueue.push(std::move(abrt));
            std::cout << "pushed abort event" << std::endl;
        }
    }
}


}; // namespace event

}; // namespace sword
