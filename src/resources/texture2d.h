#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <resource.h>
#include <aurora/gfx.h>

extern "C"{
#include "../lib/stb_image.h"
}

class Texture2D
{
public:
    Texture2D()
    : glTexName(0)
    {       
    }
    
    ~Texture2D()
    {
        glDeleteTextures(1, &glTexName);
    }
    
    void Fill(Au::GFX::Texture2D* texture)
    {
        texture->Data(_data.data(), bpp, width, height);
    }
    
    void Data(unsigned char* data, int width, int height, int bpp)
    {
        _data = std::vector<unsigned char>(data, data + bpp * width * height);
        this->width = width;
        this->height = height;
        this->bpp = bpp;
    }
    
    GLuint GetGlName()
    {
        if(glTexName == 0)
            _initGlData();
        return glTexName; 
    }
private:
    std::vector<unsigned char> _data;
    int width, height;
    int bpp;
    GLuint glTexName;
    
    void _initGlData()
    {
        glGenTextures(1, &glTexName);
        glBindTexture(GL_TEXTURE_2D, glTexName);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        GLenum format;
        if(bpp == 1) format = GL_RED;
        else if(bpp == 2) format = GL_RG;
        else if(bpp == 3) format = GL_RGB;
        else if(bpp == 4) format = GL_RGBA;
        else return;
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glTexName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, (const GLvoid*)_data.data());
    }
};

class Texture2DReaderJPG : public Resource<Texture2D>::Reader
{
public:
    Texture2D* operator()(const std::string& filename)
    {
        stbi_set_flip_vertically_on_load(1);
        int w, h, bpp;
        unsigned char* data = 
            stbi_load(filename.c_str(), &w, &h, &bpp, 3);
        if(!data)
            return 0;
     
        Texture2D* texture = new Texture2D();
        texture->Data(data, w, h, 3);
        
        return texture;
    }
};

class Texture2DReaderPNG : public Resource<Texture2D>::Reader
{
public:
    Texture2D* operator()(const std::string& filename)
    {
        stbi_set_flip_vertically_on_load(1);
        int w, h, bpp;
        unsigned char* data = 
            stbi_load(filename.c_str(), &w, &h, &bpp, 3);
        if(!data)
            return 0;
     
        Texture2D* texture = new Texture2D();
        texture->Data(data, w, h, 3);
        
        return texture;
    }
};

class Texture2DReader : public Resource<Texture2D>::Reader
{
public:
    Texture2D* operator()(const std::string& filename)
    {
        Texture2D* texture = new Texture2D();
        
        
        
        std::vector<unsigned char> data;
        data.resize(256 * 256 * 3);
        
        for(unsigned y = 0; y < 256; ++y)
        {
            for(unsigned x = 0; x < 256; ++x)
            {
                float value = (sinf(x / 8.0f) + 1.0f) / 2.0f;
                unsigned index = x + y * 256;
                
                data[index * 3] = (unsigned char)(value * 256.0f);
                data[index * 3 + 1] = (unsigned char)(value * 256.0f);
                data[index * 3 + 2] = (unsigned char)(value * 256.0f);
            }
        }
        
        texture->Data(data.data(), 256, 256, 3);
        
        return texture;
    }
};

#endif
