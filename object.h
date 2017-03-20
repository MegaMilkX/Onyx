#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <map>

#include "typeindex.h"

#include "component.h"

class Object
{
public:
    Object() : parent(0) {}
    Object(Object* parent) : parent(parent) {}
    ~Object()
    {
        for(unsigned i = 0; i < objects.size(); ++i)
            delete objects[i];
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
