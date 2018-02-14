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

class Material
{
public:    
    Material()
    {
        
    }

    std::string GetString(const std::string& key) { return "test"; }
private:
    std::map<std::string, std::string> strstr;
};

class MaterialReaderJSON : public Resource<Material>::Reader
{
public:
    Material* operator()(const std::string& filename)
    {
        Material* material = 0;
        
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if(!file.is_open())
            return 0;
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer((unsigned int)size);
        if(file.read(buffer.data(), size))
        {
            material = new Material();
            
            
        }
        
        file.close();
        
        return material;
    }
};

#endif
