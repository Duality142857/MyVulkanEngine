#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
// #include<mygeo/vec.h>
#include<mygeo/vec.h>
#include"my_event.h"

class MyWindow
{
public:
    Dispatcher dispatcher;
    MyWindow(int width, int height):width{width},height{height}
    {
        // std::cout<<"width&height "<<width<<" , "<<height<<std::endl;
        initWindow(width,height);

        dispatcher.subscribe(MouseButton_Event,[this](const Event& e){onMouseButton(e);});

        dispatcher.subscribe(MouseDrag_Event,[this](const Event& e){onMouseDrag(e);});

        dispatcher.subscribe(Key_Event,[this](const Event& e){onKey(e);});
    }

    float width, height;
    GLFWwindow* window;

    int currentKey;
    int currentKeyAction;

    float mousescrollVal=0.f;

    bool leftmousePressed=false;
    bool rightmousePressed=false;

    MyGeo::Vec2f currentCursorPosition;
    MyGeo::Vec2f lastCursorPosition;

    int currentMouseButton=-1;
    int lastMouseButton=-1;
    
    int currentMouseAction=-1;
    int lastMouseAction=-1;

    MyGeo::Vec2f leftDragVec{0,0};
    MyGeo::Vec2f rightDragVec{0,0};
    
    MyGeo::Vec2f leftStartPos{0,0};
    MyGeo::Vec2f rightStartPos{0,0};


    MyGeo::Vec3f dragAxis;
    float dragAngle;


    bool framebufferResized=false;

    bool windowShouldClose()
    {
        return glfwWindowShouldClose(window);
    }
    void pollEvents()
    {
        glfwPollEvents();
    }

    static MyGeo::Vec2f getCursorPos(GLFWwindow* window)
    {
        double x,y;
        glfwGetCursorPos(window,&x,&y);
        return {(float)x,(float)y};
    }

    MyGeo::Vec2f cursorPos()
    {
        return getCursorPos(window);
    }

    void tick()
    {
        lastCursorPosition=currentCursorPosition;
        currentCursorPosition=getCursorPos(window);
        MyGeo::Vec2f mouseDelta=currentCursorPosition-lastCursorPosition;
        // std::cout<<"mouseDelta "<<mouseDelta<<std::endl;
        
        if((leftmousePressed || rightmousePressed) && mouseDelta.norm2()>2)
        {
            MouseDragEvent e(currentMouseButton,mouseDelta);
            dispatcher.dispatch(e);
            
        }
    }

    void onKey(const Event& e)
    {
        if(e.type()==Key_Event)
        {
            const KeyEvent& event=*reinterpret_cast<const KeyEvent*>(&e);
            currentKey=event.button;
            if(event.action==GLFW_PRESS || event.action==GLFW_RELEASE)
                currentKeyAction=event.action;
            // std::cout<<"key pressed: "<<currentKey<<std::endl;
        }
    }

    void onMouseButton(const Event& e)
    {
        if(e.type()==MouseButton_Event)
        {
            const MouseButtonEvent& event=*reinterpret_cast<const MouseButtonEvent*>(&e);

            lastMouseButton=currentMouseButton;
            currentMouseButton=event.button;

            lastMouseAction=currentMouseAction;
            currentMouseAction=event.action;

            if(currentMouseButton==GLFW_MOUSE_BUTTON_LEFT && currentMouseAction==GLFW_PRESS)
            {
                // std::cout<<"left pressed!"<<std::endl;
                leftmousePressed=true;    
            }
            if(currentMouseButton==GLFW_MOUSE_BUTTON_LEFT && currentMouseAction==GLFW_RELEASE)
            {
                // std::cout<<"left released!"<<std::endl;
                leftmousePressed=false;    
            }
            if(currentMouseButton==GLFW_MOUSE_BUTTON_RIGHT && currentMouseAction==GLFW_PRESS)
            {
                // std::cout<<"right pressed!"<<std::endl;
                rightmousePressed=true;    
            }
            if(currentMouseButton==GLFW_MOUSE_BUTTON_RIGHT && currentMouseAction==GLFW_RELEASE)
            {
                // std::cout<<"right released!"<<std::endl;
                rightmousePressed=false;    
            }
            
        }
    }

    void onMouseDrag(const Event& e)
    {
        if(e.type()==MouseDrag_Event)
        {
            const MouseDragEvent& event=*reinterpret_cast<const MouseDragEvent*>(&e);
            if(event.button==GLFW_MOUSE_BUTTON_LEFT)
            {
                dragAngle=1500.f*event.dragVec.norm2()/(width*height);
                MyGeo::Vec3f dragVec_normalized=MyGeo::Vec3f{event.dragVec.x,-event.dragVec.y,0.f}.normalVec();
                dragAxis=dragVec_normalized.cross({0,0,1});
            }
            else//right 
            {
                
            }
        }
    }

    void initWindow(int width, int height)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

        glfwSetWindowUserPointer(window, &dispatcher);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

        glfwSetScrollCallback(window, scroll_callback);
        glfwSetMouseButtonCallback(window,mousebutton_callback);
        glfwSetCursorPosCallback(window,cursor_position_callback);
        glfwSetKeyCallback(window,key_callback);
        // (GLFWwindow* window, int key, int scancode, int action, int mods);
        
    }

    static void key_callback(GLFWwindow* window, int button, int scancode, int action, int mods)
    {
        KeyEvent event{button, action};
        auto tdispatcher = reinterpret_cast<Dispatcher*>(glfwGetWindowUserPointer(window));
        tdispatcher->dispatch(event);
    }

    static void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
    {
        MouseButtonEvent event{button,action,getCursorPos(window)};
        auto tdispatcher = reinterpret_cast<Dispatcher*>(glfwGetWindowUserPointer(window));
        tdispatcher->dispatch(event);
    }

    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
    {
        auto mywindow = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
        if(mywindow->leftmousePressed)
        {
            mywindow->leftDragVec=MyGeo::Vec2f{(float)xpos,(float)ypos}-mywindow->leftStartPos;
            mywindow->leftDragVec.y=-mywindow->leftDragVec.y;
        }
        if(mywindow->rightmousePressed)
        {
            mywindow->rightDragVec=MyGeo::Vec2f{(float)xpos,(float)ypos}-mywindow->rightStartPos;
            mywindow->rightDragVec.y=-mywindow->rightDragVec.y;
            mywindow->rightDragVec.x=-mywindow->rightDragVec.x;
        }
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto mywindow = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
        mywindow->mousescrollVal+=yoffset;

        // auto tdispatcher = reinterpret_cast<Dispatcher*>(glfwGetWindowUserPointer(window));
        // tdispatcher->dispatch(event);

    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto mywindow = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
        mywindow->framebufferResized = true;
    }

    void cleanup()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }


};
