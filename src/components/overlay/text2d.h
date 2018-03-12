#ifndef TEXT_2D_H
#define TEXT_2D_H

#undef GetObject

#include <scene_object.h>
#include <texture2d.h>
#include <mesh_data.h>

#include <font_data.h>
#include "../lib/nimbusmono_bold.otf.h"
#include "../../util/bitmap.h"

class OverlayRoot;
class Text2d : public SceneObject::Component
{
public:
    Text2d()
    : size(16) {}
    void SetText(const std::string& str)
    {
        SetText(std::vector<int>(str.begin(), str.end()));
    }
    void SetText(const std::vector<int>& str)
    {
        string = str;
        std::vector<float> vertices;
        std::vector<float> uvw;
        float adv = 0.0f;
        charCount = str.size();
        FontData::GlyphAtlas* atlas = fontData->GetGlyphAtlas(size);
        float line = 1.0f;
        for(unsigned i = 0; i < str.size(); ++i)
        {
            int charCode = str[i];
            if(charCode == 13 || charCode == '\n')
            {
                line += 1.0f;
                adv = 0.0f;
                continue;
            }
            FontData::Glyph* g = fontData->GetGlyph(charCode, size);
            float heightBearingDiff = g->height - g->hBearingY;
            float loff = line * atlas->metrics.lineHeight;
            vertices.insert(
                vertices.end(),
                {
                    adv + g->hBearingX + 0.0f,      (float)-g->height + heightBearingDiff + loff,               0.0f,
                    adv + g->hBearingX + g->width,  (float)-g->height + heightBearingDiff + loff,               0.0f,
                    adv + g->hBearingX + g->width,  (float)heightBearingDiff + loff,   0.0f,
                    adv + g->hBearingX + g->width,  (float)heightBearingDiff + loff,   0.0f,
                    adv + g->hBearingX + 0.0f,      (float)heightBearingDiff + loff,   0.0f,
                    adv + g->hBearingX + 0.0f,      (float)-g->height + heightBearingDiff + loff,               0.0f
                }
            );
            uvw.insert(
                uvw.end(),
                {
                    g->u0, g->v0, g->layer,
                    g->u1, g->v0, g->layer,
                    g->u1, g->v1, g->layer,
                    g->u1, g->v1, g->layer,
                    g->u0, g->v1, g->layer,
                    g->u0, g->v0, g->layer
                }
            );
            adv += g->advX;
        }
        
        mesh->SetAttribArray<Au::Position>(vertices);
        mesh->SetAttribArray<Au::UVW>(uvw);
    }

    void SetSize(unsigned sz)
    {
        size = sz;
        SetText(string);
    }

    void OnCreate()
    {
        static FontData* fd = CreateDefaultFontData();
        fontData = fd;
        mesh = new MeshData();
        GetObject()->Root()->GetComponent<OverlayRoot>();
    }
    MeshData* mesh;
    FontData* fontData;
    int charCount;
    std::vector<int> string;
    unsigned size;

    FontData* CreateDefaultFontData()
    {
        FontData* fd = new FontData();
        fd->LoadMemory((void*)nimbusmono_bold_otf, sizeof(nimbusmono_bold_otf));
        return fd;
    }
};

#endif
