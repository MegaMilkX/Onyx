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

class MaterialReaderJSON : public asset<Material>::reader
{
public:
    bool operator()(const std::string& filename, Material* material)
    {
        bool result = false;
        using json = nlohmann::json;
        
        std::ifstream file(filename, std::ios::in);
        if(!file.is_open())
            return result;
        
        json j;
        try
        {
            j = json::parse(file);
        }
        catch(std::exception& e)
        {
            std::cout << "Material json parse error: " << e.what() << std::endl;
            file.close();
            return result;
        }

        result = true;
        for(json::iterator it = j.begin(); it != j.end(); ++it)
        {
            if(it.value().is_string())
            {
                material->Set(it.key(), it.value().get<std::string>());
            }
        }
    
        
        file.close();
        
        return result;
    }
};

#endif
