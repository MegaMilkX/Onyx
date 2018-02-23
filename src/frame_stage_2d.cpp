#include "frame_stage_2d.h"

#include "components/text_mesh.h"
#include "components/camera.h"

std::string FrameStage2d::VertexShader()
{
    return R"(#version 450
        in vec3 Position;
        in vec2 UV;
        in vec3 Normal;

        out vec2 UVFrag;
        out vec3 NormalModel;
        out vec3 FragPosWorld;

        uniform mat4 MatrixModel;
        uniform mat4 MatrixView;
        uniform mat4 MatrixProjection;

        void main()
        {
            vec4 PositionScreen;
            vec4 PositionModel;
            
            PositionModel = vec4 ( Position , 1.0 ) ;
            FragPosWorld = vec3 ( MatrixModel * PositionModel ) ; 
            NormalModel = normalize ( ( MatrixModel * vec4 ( Normal , 0.0 ) ) . xyz ) ; 
            UVFrag = UV ;  
            PositionScreen = MatrixProjection * MatrixView * MatrixModel * PositionModel ; 
            gl_Position = PositionScreen ; 
        }
    )";
}
std::string FrameStage2d::PixelShader()
{
    return R"(#version 450
        #define LIGHT_DIRECT_COUNT 2
        #define LIGHT_OMNI_COUNT 2
        in vec2 UVFrag;
        in vec3 NormalModel;
        in vec3 FragPosWorld;

        out vec4 fragOut;

        uniform sampler2D DiffuseTexture;
        uniform vec3 AmbientColor;
        uniform vec3 LightDirectRGB[LIGHT_DIRECT_COUNT];
        uniform vec3 LightDirect[LIGHT_DIRECT_COUNT];
        uniform vec3 LightOmniRGB[LIGHT_OMNI_COUNT];
        uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];

        void main()
        {
            vec4 add2;
            vec4 add1;
            vec4 multiply0;
            vec4 Diffuse;
            vec4 Ambient;
            vec4 LightDirectLambert;
            vec4 LightOmniLambert;
            vec3 LightDirection[LIGHT_OMNI_COUNT];
            
            for (int i = 0; i < LIGHT_OMNI_COUNT; i++) 
            { 
                LightDirection[i] = normalize(LightOmniPos[i] - FragPosWorld); 
            } 
            
            LightOmniLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
            for (int i = 0; i < LIGHT_OMNI_COUNT; i++) 
            { 
                float diff = max(dot(NormalModel, LightDirection[i]) , 0.0); 
                float dist = distance ( LightOmniPos [ i ] , FragPosWorld ) ; 
                LightOmniLambert += vec4 ( LightOmniRGB [ i ] * diff * ( 1.0 / ( 1.0 + 0.5 * dist + 3.0 * dist * dist ) ) , 1.0 ) ; 
            } 
            
            LightDirectLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
            for (int i = 0; i < LIGHT_DIRECT_COUNT; ++i)
            { 
                float diff = max ( dot ( NormalModel , - LightDirect [ i ] ) , 0.0 ) ; 
                LightDirectLambert += vec4 ( LightDirectRGB [ i ] * diff , 1.0 ) ; 
            } 
            
            Ambient = vec4 ( AmbientColor , 1.0 ) ; 
            Diffuse = texture ( DiffuseTexture , UVFrag ) ; 
            multiply0 = Diffuse * Ambient ; 
            add1 = multiply0 + LightDirectLambert ; 
            add2 = add1 + LightOmniLambert ; 
            fragOut = add2 ; 
        }
    )";
}

void FrameStage2d::Build(SceneObject* root)
{
    textMeshes.clear();
    renderer = root->GetComponent<Renderer>();
    
    textMeshes = root->FindAllOf<TextMesh>();
}

void FrameStage2d::Run()
{
    glUseProgram(shaderProgram);
    
    Camera* cam = renderer->CurrentCamera();
    
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "MatrixProjection"), 
        1, 
        GL_FALSE, 
        (float*)&cam->Projection()
    );
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "MatrixView"), 
        1, 
        GL_FALSE, 
        (float*)&cam->InverseTransform()
    );
    
    for(TextMesh* textMesh : textMeshes)
    {
        textMesh->Render(shaderProgram);
    }
}
