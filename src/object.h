#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <map>

#include "typeindex.h"

class Object
{
public:
    class Component
    {
    friend Object;
    public:
        virtual ~Component() {}
        Object* GetParentObject() { return object; }
    private:
        Object* object;
    };
    
    Object() : parent(0) {}
    Object(Object* parent) : parent(parent) {}
    ~Object()
    {
        for(unsigned i = 0; i < objects.size(); ++i)
            delete objects[i];
    }
    
    Object* Root()
    {
        if(!parent)
            return this;
        else
            return parent->Root();
    }
    
    Object* CreateObject()
    {
        Object* o = new Object(this);
        objects.push_back(o);
        return o;
    }
    template<typename T>
    T* GetComponent()
    {
        std::map<typeindex, Component*>::iterator it;
        it = components.find(TypeInfo<T>::Index());
        if (it == components.end())
        {
            T* component = new T();
            component->object = this;
            component->OnCreate();
            components.insert(std::make_pair(TypeInfo<T>::Index(), component));
            return component;
        }
        else
            return (T*)it->second;
    }
private:
    Object* parent;
    std::vector<Object*> objects;
    std::map<typeindex, Component*> components;
};

#endif
