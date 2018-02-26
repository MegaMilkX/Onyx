#ifndef FONT_DATA_H
#define FONT_DATA_H

#include "../game_state.h"
#include <resource.h>

#include "texture2d.h"

#include "../lib/font_rasterizer.h"

class FontData
{
public:
    FontData()
    {
        rasterizer.Init();
    }
    ~FontData()
    {
        rasterizer.Cleanup();
    }
    void Load(const std::string& filename)
    {
        rasterizer.ReadFile(filename);
    }
    void LoadMemory(void* data, size_t sz)
    {
        rasterizer.ReadMemory(data, sz);
    }
    void AddChar(unsigned charCode, unsigned size)
    {
        rasterizer.AddChar(charCode, size);
    }
    void RebuildTexture()
    {
        FontRasterizer::Bitmap bmp;
        rasterizer.Rasterize(bmp);
        texture.Data((unsigned char *)bmp.data, bmp.width, bmp.height, bmp.bpp);
        bmp.Free();
    }
    Texture2D* GetTexture() { return &texture; }
    GlyphInfo* GetGlyph(unsigned charCode) 
    { 
        glyphs[charCode] = rasterizer.GetGlyph(charCode);
        return &glyphs[charCode]; 
    }
private:
    std::map<unsigned, GlyphInfo> glyphs;
    Texture2D texture;
    FontRasterizer rasterizer;
};

class FontDataReader : public resource<FontData>::reader
{
public:
    FontData* operator()(const std::string& filename)
    {
        FontData* fontData = 0;
        fontData = new FontData();
        fontData->Load(filename);
        return fontData;
    }
};

#endif
