#ifndef FONT_DATA_H
#define FONT_DATA_H

#include "../game_state.h"
#include <resource.h>

#include "texture2d.h"
#include "texture3d.h"

#include "../lib/font_rasterizer.h"

#define GLYPH_PAGE_SIZE 256

class FontData
{
public:
    struct Glyph
    {
        int width, height;
        int advX;
        int hBearingY;
        float u0, v0, u1, v1;
        float layer;
    };
    struct GlyphAtlas
    {
        GlyphAtlas()
        : slot(0, 0, 0), pages(1) 
        {
            
        }

        FontRasterizer::GlobalMetrics metrics;
        Au::Math::Vec2i glyphPerPage;
        Au::Math::Vec3i slot;
        Texture3D* texture;
        unsigned char* data;
        int pages;
        std::map<unsigned, Glyph> glyphs;
        bool Exists(unsigned charCode)
        {
            return glyphs.count(charCode) != 0;
        }
        void AddPage()
        {
            pages++;
            delete[] data;
            data = new unsigned char[GLYPH_PAGE_SIZE * GLYPH_PAGE_SIZE * pages];
            unsigned char* old = texture->GetData();
            memcpy(data, old, GLYPH_PAGE_SIZE * GLYPH_PAGE_SIZE * (pages - 1));
            texture->Data(data, GLYPH_PAGE_SIZE, GLYPH_PAGE_SIZE, pages, 1);
        }
    };
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
    void RebuildTexture()
    {
        FontRasterizer::Bitmap bmp;
        rasterizer.Rasterize(bmp);
        texture.Data((unsigned char *)bmp.data, bmp.width, bmp.height, bmp.bpp);
        bmp.Free();
    }
    void RebuildMesh()
    {

    }
    GlyphAtlas* GetGlyphAtlas(unsigned size) 
    {
        if(atlases.count(size) == 0)
        {
            atlases[size].texture = new Texture3D();
            atlases[size].data = new unsigned char[GLYPH_PAGE_SIZE * GLYPH_PAGE_SIZE];
            atlases[size].texture->Data(atlases[size].data, GLYPH_PAGE_SIZE, GLYPH_PAGE_SIZE, 1, 1);
        }
        GlyphAtlas& a = atlases[size];
        a.metrics = rasterizer.GetGlobalMetrics(size);
        a.glyphPerPage.x = GLYPH_PAGE_SIZE / a.metrics.bbox.x;
        a.glyphPerPage.y = GLYPH_PAGE_SIZE / a.metrics.bbox.y;
        return &atlases[size]; 
    }
    Texture2D* GetTexture() { return &texture; }
    Glyph* GetGlyph(unsigned charCode, unsigned size) 
    {
        GlyphAtlas* atlas = GetGlyphAtlas(size);
        FontRasterizer::GlyphInfo* g = 
            rasterizer.GetGlyph(charCode, size);
        if(!atlas->Exists(charCode))
        {
            atlas->texture->Blit2d(
                (unsigned char*)g->bitmap.data,
                g->bitmap.width, g->bitmap.height, 1,
                atlas->slot.x * atlas->metrics.bbox.x, 
                atlas->slot.y * atlas->metrics.bbox.y,
                atlas->slot.z
            );
            atlas->glyphs[charCode] = 
                Glyph{
                    (int)g->width, (int)g->height,
                    (int)g->advX,
                    (int)g->hBearingY,
                    (float)(atlas->slot.x * atlas->metrics.bbox.x) / (float)GLYPH_PAGE_SIZE, 
                    (float)(atlas->slot.y * atlas->metrics.bbox.y) / (float)GLYPH_PAGE_SIZE, 
                    (float)(atlas->slot.x * atlas->metrics.bbox.x) / (float)GLYPH_PAGE_SIZE + 
                    (float)g->bitmap.width / (float)GLYPH_PAGE_SIZE, 
                    (float)(atlas->slot.y * atlas->metrics.bbox.y) / (float)GLYPH_PAGE_SIZE + 
                    (float)g->bitmap.height / (float)GLYPH_PAGE_SIZE,
                    (float)atlas->slot.z
                };

            atlas->slot.x += 1;
            if(atlas->slot.x >= atlas->glyphPerPage.x)
            {
                atlas->slot.x = 0;
                atlas->slot.y += 1;
            }
            if(atlas->slot.y >= atlas->glyphPerPage.y)
            {
                atlas->slot.x = 0;
                atlas->slot.y = 0;
                atlas->slot.z += 1;
                atlas->AddPage();
            }
        }
        return &atlas->glyphs[charCode];
    }
private:
    std::map<unsigned, GlyphAtlas> atlases;
    Texture2D texture;
    FontRasterizer rasterizer;
};

class FontDataReader : public asset<FontData>::reader
{
public:
    bool operator()(const std::string& filename, FontData* fontData)
    {
        fontData->Load(filename);
        return true;
    }
};

#endif
