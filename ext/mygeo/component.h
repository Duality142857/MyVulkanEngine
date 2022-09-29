#include<map>
#include<string>
#include<typeinfo>
#include<iostream>
#include<memory>
struct Component
{
    std::string typeString;
};

struct Foo: public Component
{
    Foo()
    {
        typeString=typeid(Foo).name();
    }
    int f;

};

struct Bar: public Component
{
    Bar()
    {
        typeString=typeid(Bar).name();
    }
    int b;
};

struct Gun: public Component
{
    Gun()
    {
        typeString=typeid(Gun).name();
        std::cout<<typeString<<std::endl;
    }
    std::string getTypeString() const
    {
        return typeid(Gun).name();
    }
    void shot()
    {
        std::cout<<"shot!"<<std::endl;
    }
};

class Entity
{
public:
    std::map<std::string,std::shared_ptr<Component>> componentMap;
    void addComponent(std::shared_ptr<Component> component)
    {
        if(componentMap.find(component->typeString)==componentMap.end()) 
        {
            componentMap[component->typeString]=component;
        }
        else std::cout<<"component already exist!"<<std::endl;
    }
    template<class T>
    std::shared_ptr<T> getComponent()
    {
        if(componentMap.find(std::string(typeid(T).name()))!=componentMap.end())
        {
            return std::static_pointer_cast<T>(componentMap[std::string(typeid(T).name())]);
        }
        else 
        {
            return nullptr;
        }
    }
};


