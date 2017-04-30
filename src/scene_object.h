#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <vector>
#include <map>

#include "typeindex.h"

class SceneObject
{
public:
    class Component
    {
    friend SceneObject;
    public:
        virtual ~Component() {}
        SceneObject* GetObject() { return object; }
    private:
        SceneObject* object;
    };
    
    SceneObject() : parentObject(0) {}
    SceneObject(SceneObject* parent) : parentObject(parent) {}
    ~SceneObject()
    {
        for(unsigned i = 0; i < objects.size(); ++i)
            delete objects[i];
    }
    
    SceneObject* Root()
    {
        if(!parentObject)
            return this;
        else
            return parentObject->Root();
    }
    
    SceneObject* CreateObject()
    {
        SceneObject* o = new SceneObject(this);
        objects.push_back(o);
        return o;
    }
    template<typename T>
    T* GetComponent()
    {
        T* c = FindComponent<T>();
        if (!c)
        {
            c = new T();
            c->object = this;
            components.insert(std::make_pair(TypeInfo<T>::Index(), c));
            c->OnCreate();
            return c;
        }
        else
            return c;
    }
    
    template<typename T>
    T* FindComponent()
    {
        std::map<typeindex, Component*>::iterator it;
        it = components.find(TypeInfo<T>::Index());
        if(it == components.end())
            return 0;
        else
            return (T*)it->second;
    }
private:
    SceneObject* parentObject;
    std::vector<SceneObject*> objects;
    std::map<typeindex, Component*> components;
};

#endif
