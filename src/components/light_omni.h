#ifndef COMPONENT_LIGHT_OMNI_H
#define COMPONENT_LIGHT_OMNI_H

#include "../scene_object.h"
#include "renderer.h"

#include <aurora/math.h>

class LightDirect : public SceneObject::Component
{
public:
    ~LightDirect()
    {
        GetObject()->Root()->GetComponent<Renderer>()->RemoveLightDirect(this);
    }
    
    void Color(float r, float g, float b)
    { color = Au::Math::Vec3f(r, g, b); }
    
    Au::Math::Vec3f Color()
    { return color; }
    
    void Direction(float x, float y, float z)
    { direction = Au::Math::Vec3f(x, y, z); }
    Au::Math::Vec3f Direction()
    { return direction; }
    
    void OnCreate()
    {
        GetObject()->Root()->GetComponent<Renderer>()->AddLightDirect(this);
    }
    virtual std::string Serialize() 
    { 
        using json = nlohmann::json;
        json j = json::object();
        j["Color"] = {color.x, color.y, color.z};
        j["Direction"] = {direction.x, direction.y, direction.z};
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Color"].is_array() && j["Color"].size() == 3)
        {
            Color(
                j["Color"][0].get<float>(),
                j["Color"][1].get<float>(),
                j["Color"][2].get<float>()
            );
        }
        if(j["Direction"].is_array() && j["Direction"].size() == 3)
        {
            Direction(
                j["Direction"][0].get<float>(),
                j["Direction"][1].get<float>(),
                j["Direction"][2].get<float>()
            );
        }
    }
private:
    Au::Math::Vec3f color;
    Au::Math::Vec3f direction;
};

class LightOmni : public SceneObject::Component
{
public:
    ~LightOmni()
    {
        GetObject()->Root()->GetComponent<Renderer>()->RemoveLightOmni(this);
    }

    void Color(float r, float g, float b)
    {
        color = Au::Math::Vec3f(r, g, b);
    }
    void Intensity(float i)
    {
        intensity = i;
    }
    
    Au::Math::Vec3f Color()
    { return color; }

    void OnCreate()
    {
        GetObject()->Root()->GetComponent<Renderer>()->AddLightOmni(this);
    }
    
    virtual std::string Serialize() 
    { 
        using json = nlohmann::json;
        json j = json::object();
        j["Color"] = {color.x, color.y, color.z};
        j["Intensity"] = intensity;
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Color"].is_array() && j["Color"].size() == 3)
        {
            Color(
                j["Color"][0].get<float>(),
                j["Color"][1].get<float>(),
                j["Color"][2].get<float>()
            );
        }
        if(j["Intensity"].is_number())
        {
            Intensity(j["Intensity"].get<float>());
        }
    }
private:
    Au::Math::Vec3f color;
    float intensity;
};

#endif
