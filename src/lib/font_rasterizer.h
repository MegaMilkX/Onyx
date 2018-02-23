#ifndef FONT_RASTERIZER_H
#define FONT_RASTERIZER_H

#include <iostream>

#include "bin_packer_2d.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct GlyphInfo
{
    unsigned charCode;
    unsigned glyphCode;
    unsigned size;
    float width, height;
    float advX, advY;
    float vBearingX, vBearingY;
    float hBearingX, hBearingY;
};

class FontRasterizer
{
public:
    FontRasterizer()
    : ftLib(0), face(0) {}
    
    void Init()
    {
        FT_Init_FreeType(&ftLib);
    }
    void Cleanup()
    {
        if(face)
            FT_Done_Face(face);
        if(ftLib)
            FT_Done_FreeType(ftLib);
    }
    
    bool ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if(!file.is_open())
            return false;
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        buffer = std::vector<char>((unsigned int)size);
        if(file.read(buffer.data(), size))
        {
            return ReadMemory(buffer.data(), size);
        }
        else
        {
            return false;
        }
        
        return true;
    }
    
    bool ReadMemory(void* data, size_t sz)
    {
        if(FT_New_Memory_Face(ftLib, (FT_Byte*)data, sz, 0, &face))
        {
            std::cout << "FT_New_Memory_Face failed" << std::endl;
            return false;
        }
        return true;
    }
    
    struct Bitmap
    {
        Bitmap()
        : data(0), width(0), height(0), bpp(0)
        {}
        
        void Free()
        {
            if(data)
                free(data);
            *this = Bitmap();
        }
        
        char* data;
        unsigned width;
        unsigned height;
        unsigned bpp;
    };
    
    void AddChar(unsigned int charCode, unsigned int size)
    {
        glyphs.push_back({charCode, 0, size});
    }
    
    GlyphInfo GetGlyph(unsigned charCode)
    {
        unsigned glyphCode = FT_Get_Char_Index(face, charCode);
        FT_Load_Glyph(face, glyphCode, FT_LOAD_DEFAULT | FT_LOAD_TARGET_NORMAL);
        GlyphInfo gi;
        gi.charCode = charCode;
        gi.glyphCode = glyphCode;
        gi.size = 1;
        gi.width = face->glyph->metrics.width;
        gi.height = face->glyph->metrics.height;
        gi.advX = face->glyph->metrics.horiAdvance;
        gi.advY = face->glyph->metrics.vertAdvance;
        return gi;
    }
    
    Bitmap Rasterize(unsigned int charCode, unsigned int faceSize)
    {
        Bitmap bmp;
        if(!ftLib)
        {
            std::cout << "FreeType lib is not initialized" << std::endl;
            return bmp;
        }
        if(!face)
        {
            std::cout << "No FreeType face" << std::endl;
            return bmp;
        }
        
        if(FT_Set_Char_Size(face, 0, faceSize * 64, 72, 72))
        {
            std::cout << "FT_Set_Char_Size failed" << std::endl;
            return bmp;
        }
        
        unsigned int glyphCode = FT_Get_Char_Index(face, charCode);
        
        if(FT_Load_Glyph(face, glyphCode, FT_LOAD_DEFAULT | FT_LOAD_TARGET_NORMAL))
        {
            std::cout << "FT_Load_Glyph failed" << std::endl;
            return bmp;
        }
        
        if(face->glyph->format!=FT_GLYPH_FORMAT_BITMAP)
		{
			//There is no bitmap for that glyph. Rendering.
			if(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
			{
				//Glyph could not be rendered
                std::cout << "FT_Render_Glyph failed" << std::endl;
                return bmp;
            }
        }
        
        switch(face->glyph->bitmap.pixel_mode)
        {
        case FT_PIXEL_MODE_NONE: break;
        case FT_PIXEL_MODE_MONO: break;
        case FT_PIXEL_MODE_GRAY:
            bmp.data = (char*)malloc(face->glyph->bitmap.width * face->glyph->bitmap.rows);
            memcpy(bmp.data, face->glyph->bitmap.buffer, face->glyph->bitmap.width*face->glyph->bitmap.rows);
            bmp.bpp = 1;
            bmp.width = face->glyph->bitmap.width;
            bmp.height = face->glyph->bitmap.rows;
            break;
        case FT_PIXEL_MODE_GRAY2: break;
        case FT_PIXEL_MODE_GRAY4: break;
        case FT_PIXEL_MODE_LCD: break;
        case FT_PIXEL_MODE_LCD_V: break;
        case FT_PIXEL_MODE_BGRA: break;
        }
        
        return bmp;
    }
    
    void BmpBlit(
        void* destData,
        unsigned destW,
        unsigned destH,
        unsigned destBpp,
        void* srcData, 
        unsigned srcW, 
        unsigned srcH,
        unsigned srcBpp,
        unsigned xOffset,
        unsigned yOffset)
    {
        unsigned char* dest = (unsigned char*)destData;
        unsigned char* src = (unsigned char*)srcData;
        for(unsigned y = 0; (y + yOffset) < destH && y < srcH; ++y)
        {
            for(unsigned x = 0; (x + xOffset) < destW && x < srcW; ++x)
            {
                unsigned index = ((y + yOffset) * destW + (x + xOffset)) * destBpp;
                unsigned srcIndex = (y * srcW + x) * srcBpp;
                for(unsigned b = 0; b < destBpp && b < srcBpp; ++b)
                {
                    dest[index + b] = src[srcIndex + b];
                }
                if(destBpp == 4 && srcBpp < 4)
                    dest[index + 3] = 255;
            }
        }
    }
    
    void BmpBlit(Bitmap& dest, Bitmap& src, unsigned xOffset, unsigned yOffset)
    {
        BmpBlit(
            dest.data, 
            dest.width, 
            dest.height, 
            dest.bpp,
            src.data,
            src.width,
            src.height,
            src.bpp,
            xOffset,
            yOffset
        );
    }
    
    void Rasterize(Bitmap& bmp)
    {
        bp2D::BinPacker2D packer;
        std::vector<Bitmap> bmps;
        for(GlyphInfo glyph : glyphs)
        {
            Bitmap bmp = Rasterize(glyph.charCode, glyph.size);
            packer.AddRect(bp2D::BinRect(bmps.size(), bmp.width, bmp.height));
            bmps.push_back(bmp);
        }
        packer.Pack(BINPACKPARAM_POWEROFTWO, bp2D::SORT_MAXSIDE);
        std::vector<bp2D::BinRect> rects = packer.GetVolumes();
        bp2D::BinRect rootRect = packer.GetRootRect();
        
        bmp.data = (char*)malloc(rootRect.w * rootRect.h);
        bmp.width = rootRect.w;
        bmp.height = rootRect.h;
        bmp.bpp = 1;
        
        for(bp2D::BinRect rect : rects)
        {
            BmpBlit(bmp, bmps[rect.id], rect.x, rect.y);
        }
    }
private:
    FT_Library ftLib;
    FT_Face face;
    std::vector<char> buffer;
    
    std::vector<GlyphInfo> glyphs;
};

#endif
