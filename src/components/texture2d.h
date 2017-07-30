#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include "../resource.h"
#include <aurora/gfx.h>

class Texture2D
{
public:
    Texture2D()
    : texture(0)
    {
        
    }
    
    ~Texture2D()
    {
        if(texture && _device)
            _device->Destroy(texture);
    }
    
    void Data(unsigned char* data, int width, int height, int bpp)
    {
        _data = std::vector<unsigned char>(data, data + bpp * width * height);
        this->width = width;
        this->height = height;
        this->bpp = bpp;
    }
    
    void Finalize(Au::GFX::Device* device)
    {
        texture = device->CreateTexture2D();
        texture->Data(_data.data(), bpp, width, height);
        _device = device;
    }
    
    void Bind(int layer = 0)
    {
        texture->Bind(layer);
    }
private:
    Au::GFX::Texture2D* texture;
    Au::GFX::Device* _device;
    
    std::vector<unsigned char> _data;
    int width, height;
    int bpp;
};

class Texture2DReader : public Resource<Texture2D>::Reader
{
public:
    Texture2D* operator()(const std::string& filename)
    {
        Texture2D* texture = new Texture2D();
        
        std::vector<unsigned char> data;
        data.resize(256 * 256 * 3);
        for(unsigned i = 0; i < 256 * 256 * 3; i+=3)
        {
            data[i] = 100;
            data[i+1] = 255;
            data[i+2] = 0;
        }
        
        texture->Data(data.data(), 256, 256, 3);
        
        return texture;
    }
};

#endif