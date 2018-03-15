#ifndef OVERLAY_ROOT_H
#define OVERLAY_ROOT_H

#undef GetObject

#include <mutex>

#include <transform.h>
#include <renderer.h>
#include <scene_object.h>
#include <asset.h>
#include <util/gl_render_state.h>
#include <mesh_data.h>

#include "quad.h"
#include "text2d.h"

struct GuiDrawData
{
    resource<gl::ShaderProgram> program;
    resource<gl::ShaderProgram> program_text;
    GLuint vao;
    std::vector<Quad*> quads;
    std::vector<Text2d*> texts;
};

inline void fg_GuiBuild(const FrameCommon& frame, GuiDrawData& out)
{
    out.quads = frame.scene->FindAllOf<Quad>();
    out.texts = frame.scene->FindAllOf<Text2d>();
}

inline void fg_GuiDraw(const FrameCommon& frame, const GuiDrawData& in)
{
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    in.program->Use();    
    glUniformMatrix4fv(
        in.program->GetUniform("MatrixProjection"), 1, GL_FALSE,
        (float*)&Au::Math::Ortho(0, frame.width, 0, frame.height, -1.0f, 1.0f)
    );    
    glUniformMatrix4fv(
        in.program->GetUniform("MatrixView"), 1, GL_FALSE,
        (float*)&frame.view
    );    
    for(Quad* quad : in.quads)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, quad->image->GetGlName());
        glUniformMatrix4fv(
            in.program->GetUniform("MatrixModel"), 1, GL_FALSE,
            (float*)&quad->GetComponent<Transform>()->GetTransform()
        );
        glUniform2f(
            in.program->GetUniform("QuadSize"),
            quad->width,
            quad->height
        );
        glBindVertexArray(in.vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    in.program_text->Use();
    glUniformMatrix4fv(
        in.program_text->GetUniform("MatrixProjection"), 1, GL_FALSE,
        (float*)&Au::Math::Ortho(0, frame.width, frame.height, 0, -1.0f, 1.0f)
    );
    glUniformMatrix4fv(
        in.program_text->GetUniform("MatrixView"), 1, GL_FALSE,
        (float*)&frame.view
    );
    for(Text2d* text : in.texts)
    {
        glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, text->texture->GetGlName());
        glBindTexture(GL_TEXTURE_3D, text->fontData->GetGlyphAtlas(text->size)->texture->GetGlName());
        glUniformMatrix4fv(
            in.program_text->GetUniform("MatrixModel"), 1, GL_FALSE,
            (float*)&text->GetComponent<Transform>()->GetTransform()
        );
        glUniform1f(
            in.program_text->GetUniform("GlyphPageCount"),
            (float)text->fontData->GetGlyphAtlas(text->size)->pages
        );

        glBindVertexArray(text->mesh->GetVao({
            { "Position", 3, GL_FLOAT, GL_FALSE },
            { "UVW", 3, GL_FLOAT, GL_FALSE }
        }));
        glDrawArrays(GL_TRIANGLES, 0, text->charCount * 6);
    }
}

class OverlayRoot : public SceneObject::Component
{
public:
    void OnCreate()
    {
        renderer = GetObject()->GetComponent<Renderer>();

        static std::once_flag once_flag;
        std::call_once(
            once_flag,
            [this](){
                resource<gl::ShaderProgram> prog = 
                    resource<gl::ShaderProgram>::get("gui_shader");
                gl::Shader vs;
                gl::Shader fs;
                vs.Init(GL_VERTEX_SHADER);
                vs.Source(
                    #include "../shaders/gui_vs.glsl"
                );
                vs.Compile();
                fs.Init(GL_FRAGMENT_SHADER);
                fs.Source(
                    #include "../shaders/gui_fs.glsl"
                );
                fs.Compile();
                prog->AttachShader(&vs);
                prog->AttachShader(&fs);
                prog->BindAttrib(0, "Position");
                prog->BindAttrib(1, "UV");
                prog->BindFragData(0, "fragOut");
                prog->Link();
                prog->Use();
                glUniform1i(prog->GetUniform("DiffuseTexture"), 0);
                // ===
                resource<gl::ShaderProgram> prog_text = 
                    resource<gl::ShaderProgram>::get("gui_text_shader");
                gl::Shader vst;
                gl::Shader fst;
                vst.Init(GL_VERTEX_SHADER);
                vst.Source(
                    #include "../shaders/text_vs.glsl"
                );
                vst.Compile();
                fst.Init(GL_FRAGMENT_SHADER);
                fst.Source(
                    #include "../shaders/text_fs.glsl"
                );
                fst.Compile();
                prog_text->AttachShader(&vst);
                prog_text->AttachShader(&fst);
                prog_text->BindAttrib(0, "Position");
                prog_text->BindAttrib(1, "UVW");
                prog_text->BindFragData(0, "fragOut");
                prog_text->Link();
                prog_text->Use();
                glUniform1i(prog_text->GetUniform("GlyphAtlas"), 0);
                glUniform1i(prog_text->GetUniform("DiffuseTexture"), 0);
                // ===
                resource<MeshData> quad = 
                    resource<MeshData>::get("$quad_mesh");
                std::vector<float> vertices = {
                    0.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f
                };
                std::vector<float> uv = {
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f
                };
                quad->SetAttribArray<Au::Position>(vertices);
                quad->SetAttribArray<Au::UV>(uv);
            }
        );

        task_graph::graph& fg = renderer->GetFrameGraph();

        GuiDrawData gdd{ 
            resource<gl::ShaderProgram>::get("gui_shader"),
            resource<gl::ShaderProgram>::get("gui_text_shader"),
            resource<MeshData>::get("$quad_mesh")->GetVao({
                { "Position", 3, GL_FLOAT, GL_FALSE },
                { "UV", 2, GL_FLOAT, GL_FALSE }
            })
        };
        fg.set_data(gdd);

        fg += task_graph::once(fg_GuiBuild);
        fg.reset_once_flag(fg_GuiBuild);
        fg += fg_GuiDraw;
    }
private:
    Renderer* renderer;
};

#endif
