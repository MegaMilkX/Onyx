#ifndef COMPONENT_MATERIAL_H
#define COMPONENT_MATERIAL_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>

#include <aurora/glslstitch.h>
#include <aurora/lua.h>

#include "../scene_object.h"
#include "renderer.h"

#include <resource.h>

#include "texture2d.h"

#include "../external/json.hpp"

class Material
{
public:    
    Material()
    {
        
    }
    
    void Set(const std::string& key, const std::string& value)
    {
        strstr[key] = value;
    }

    std::string GetString(const std::string& key) { return strstr[key]; }
private:
    std::map<std::string, std::string> strstr;
};

class MaterialReaderJSON : public resource<Material>::reader
{
public:
    Material* operator()(const std::string& filename)
    {
        using json = nlohmann::json;
        Material* material = 0;
        
        std::ifstream file(filename, std::ios::in);
        if(!file.is_open())
            return 0;
        
        json j;
        try
        {
            j = json::parse(file);
        }
        catch(std::exception& e)
        {
            std::cout << "Material json parse error: " << e.what() << std::endl;
            file.close();
            return 0;
        }

        material = new Material();
        for(json::iterator it = j.begin(); it != j.end(); ++it)
        {
            if(it.value().is_string())
            {
                material->Set(it.key(), it.value().get<std::string>());
            }
        }
    
        
        file.close();
        
        return material;
    }
};

#endif
