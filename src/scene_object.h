#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <vector>
#include <map>

#include "typeindex.h"

#undef GetObject

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
    
    bool IsRoot()
    {
        return this == Root();
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
    
    void Name(const std::string& name) { this->name = name; }
    std::string Name() { return name; }
    
    SceneObject* FindObject(const std::string& name)
    {
        SceneObject* o = 0;
        for(unsigned i = 0; i < objects.size(); ++i)
        {
            if(objects[i]->Name() == name)
            {
                o = objects[i];
                break;
            }
            else if(o = objects[i]->FindObject(name))
            {
                break;
            }
        }
        return o;
    }
private:
    std::string name;
    SceneObject* parentObject;
    std::vector<SceneObject*> objects;
    std::map<typeindex, Component*> components;
};

#endif
