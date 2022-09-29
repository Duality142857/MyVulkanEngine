#pragma once

#include<functional>
#include<GLFW/glfw3.h>
#include<mygeo/vec.h>
#include<map>

enum EventType
{
    WindowResized_Event=0,MouseButton_Event,MouseDrag_Event
};



class Event
{
public:
    // using EventType=const char*;
    virtual EventType type() const =0;
};

class WindowResizedEvent: public Event
{
public:
    // WindowResizedEvent(int width, int height):width{width},height{height}
    // {}
    WindowResizedEvent(){}
    EventType type() const override { return Type;}
    static constexpr EventType Type=WindowResized_Event;

    int getWidth(){return width;}
    int getHeight(){return height;}
    int width, height;
};

class MouseButtonEvent: public Event 
{
public:
    int button=GLFW_MOUSE_BUTTON_LEFT;
    int action=GLFW_PRESS;
    MyGeo::Vec2f cursorPosition;

    static constexpr EventType Type=MouseButton_Event;
    EventType type() const override { return Type;}

    MouseButtonEvent(int _button,int _action, const MyGeo::Vec2f& cursorPosition):button{_button},action{_action}, cursorPosition{cursorPosition}{}
};

class MouseDragEvent: public Event 
{
public:
    int button=GLFW_MOUSE_BUTTON_LEFT;
    MyGeo::Vec2f dragVec;
    static constexpr EventType Type=MouseDrag_Event;
    EventType type() const override { return Type;}
    MouseDragEvent(int _button, const MyGeo::Vec2f& _dragVec):button{_button},dragVec{_dragVec}{}
};


class Dispatcher
{
public:
    using SlotType=std::function<void(const Event&)>;
    void subscribe(const EventType& etype , SlotType&& slot)
    {
        observers[etype].emplace_back(slot);
    }
    void dispatch(const Event& event)
    {
        if(observers.find(event.type())==observers.end()) return;
        auto&& valid_observers=observers[event.type()];
        for(auto&& observer:valid_observers)
        {
            observer(event);
        }
    }
private:
    std::map<EventType,std::vector<SlotType>> observers;

};
