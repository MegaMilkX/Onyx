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
        SceneObject* GetParentObject() { return object; }
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
    
    SceneObject* CreateSceneObject()
    {
        SceneObject* o = new SceneObject(this);
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
    SceneObject* parentObject;
    std::vector<SceneObject*> objects;
    std::map<typeindex, Component*> components;
};

#endif
