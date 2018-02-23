#include "frame_stage_skin.h"

#include "components/skin_mesh.h"
#include "components/skeleton.h"
#include "components/camera.h"
#include "components/light_omni.h"

std::string FrameStageSkin::VertexShader()
{
    return R"(#version 450
        #define MAX_BONE_COUNT 32
        in vec3 Position;
        in vec4 BoneIndex4;
        in vec4 BoneWeight4;
        in vec2 UV;
        in vec3 Normal;

        out vec2 UVFrag;
        out vec3 NormalModel;
        out vec3 FragPosWorld;

        uniform mat4 MatrixModel;
        uniform mat4 MatrixView;
        uniform mat4 MatrixProjection;
        uniform mat4 BoneInverseBindTransforms[MAX_BONE_COUNT];
        uniform mat4 BoneTransforms[MAX_BONE_COUNT];

        void main()
        {
        vec4 PositionScreen;
        vec4 PositionModel;
        mat4 MatrixSkin;
        {
        {
        int bi0 = int ( BoneIndex4 . x ) ; 
        int bi1 = int ( BoneIndex4 . y ) ; 
        int bi2 = int ( BoneIndex4 . z ) ; 
        int bi3 = int ( BoneIndex4 . w ) ; 
        MatrixSkin = BoneTransforms [ bi0 ] * BoneInverseBindTransforms [ bi0 ] * BoneWeight4 . x + BoneTransforms [ bi1 ] * BoneInverseBindTransforms [ bi1 ] * BoneWeight4 . y + BoneTransforms [ bi2 ] * BoneInverseBindTransforms [ bi2 ] * BoneWeight4 . z + BoneTransforms [ bi3 ] * BoneInverseBindTransforms [ bi3 ] * BoneWeight4 . w ; 
        }
        {
        PositionModel = MatrixSkin * vec4 ( Position , 1.0 ) ; 
        }
        {
        PositionScreen = MatrixProjection * MatrixView * MatrixModel * PositionModel ; 
        }
        gl_Position = PositionScreen ; 
        }
        {
        FragPosWorld = vec3 ( MatrixModel * PositionModel ) ; 
        }
        {
        NormalModel = normalize ( ( MatrixModel * MatrixSkin * vec4 ( Normal , 0.0 ) ) . xyz ) ; 
        }
        {
        UVFrag = UV ; 
        }
        {
        int bi0 = int ( BoneIndex4 . x ) ; 
        int bi1 = int ( BoneIndex4 . y ) ; 
        int bi2 = int ( BoneIndex4 . z ) ; 
        int bi3 = int ( BoneIndex4 . w ) ; 
        MatrixSkin = BoneTransforms [ bi0 ] * BoneInverseBindTransforms [ bi0 ] * BoneWeight4 . x + BoneTransforms [ bi1 ] * BoneInverseBindTransforms [ bi1 ] * BoneWeight4 . y + BoneTransforms [ bi2 ] * BoneInverseBindTransforms [ bi2 ] * BoneWeight4 . z + BoneTransforms [ bi3 ] * BoneInverseBindTransforms [ bi3 ] * BoneWeight4 . w ; 
        }
        {
        PositionModel = MatrixSkin * vec4 ( Position , 1.0 ) ; 
        }
        {
        PositionScreen = MatrixProjection * MatrixView * MatrixModel * PositionModel ; 
        }
        gl_Position = PositionScreen ; 
        }
    )";
}
std::string FrameStageSkin::PixelShader()
{
    return R"(#version 450
        #define LIGHT_DIRECT_COUNT 2
        #define LIGHT_OMNI_COUNT 2
        in vec2 UVFrag;
        in vec3 NormalModel;
        in vec3 FragPosWorld;

        out vec4 fragOut;

        uniform sampler2D DiffuseTexture;
        uniform vec3 RimColor;
        uniform mat4 MatrixView;
        uniform vec3 LightDirectRGB[LIGHT_DIRECT_COUNT];
        uniform vec3 LightDirect[LIGHT_DIRECT_COUNT];
        uniform vec3 LightOmniRGB[LIGHT_OMNI_COUNT];
        uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];

        void main()
        {
        vec4 add2;
        vec4 add1;
        vec4 add0;
        vec4 Diffuse;
        vec4 RimLight;
        vec4 LightDirectLambert;
        vec4 LightOmniLambert;
        vec3 LightDirection[LIGHT_OMNI_COUNT];
        {
        for ( int i = 0 ; 
        i < LIGHT_OMNI_COUNT ; 
        i ++ ) { LightDirection [ i ] = normalize ( LightOmniPos [ i ] - FragPosWorld ) ; 
        } }
        {
        LightOmniLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
        for ( int i = 0 ; 
        i < LIGHT_OMNI_COUNT ; 
        i ++ ) { float diff = max ( dot ( NormalModel , LightDirection [ i ] ) , 0.0 ) ; 
        float dist = distance ( LightOmniPos [ i ] , FragPosWorld ) ; 
        LightOmniLambert += vec4 ( LightOmniRGB [ i ] * diff * ( 1.0 / ( 1.0 + 0.5 * dist + 3.0 * dist * dist ) ) , 1.0 ) ; 
        } }
        {
        LightDirectLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
        for ( int i = 0 ; 
        i < LIGHT_DIRECT_COUNT ; 
        ++ i ) { float diff = max ( dot ( NormalModel , - LightDirect [ i ] ) , 0.0 ) ; 
        LightDirectLambert += vec4 ( LightDirectRGB [ i ] * diff , 1.0 ) ; 
        } }
        {
        vec3 camPos = inverse ( MatrixView ) [ 3 ] . xyz ; 
        float diff = max ( 0.5 - dot ( NormalModel , normalize ( camPos - FragPosWorld ) ) , 0.0 ) ; 
        RimLight = vec4 ( RimColor * diff , 1.0 ) ; 
        }
        {
        Diffuse = texture ( DiffuseTexture , UVFrag ) ; 
        }
        {
        add0 = Diffuse + RimLight ; 
        }
        {
        add1 = add0 + LightDirectLambert ; 
        }
        {
        add2 = add1 + LightOmniLambert ; 
        }
        fragOut = add2 ; 
        }
    )";
}

void FrameStageSkin::Build(SceneObject* root)
{
    renderer = root->GetComponent<Renderer>();
    
    std::vector<SkinMesh*> meshes = root->FindAllOf<SkinMesh>();
    for(SkinMesh* mesh : meshes)
    {
        if(!mesh->GetMeshData())
            continue;
        RenderUnit unit;
        unit.Init();
        unit.transform = mesh->GetComponent<Transform>();
        unit.FillMesh(mesh->GetMeshData());
        unit.diffuse = new Au::GFX::Texture2D();
        Resource<Texture2D>::Get(mesh->GetMaterial()->GetString("Diffuse"))->Fill(unit.diffuse);
        
        unit.skeleton = mesh->GetComponent<Skeleton>();
        
        units.push_back(unit);
    }
    
    lightsOmni = root->FindAllOf<LightOmni>();
    lightsDirect = root->FindAllOf<LightDirect>();
}
void FrameStageSkin::Run()
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
    
    for(unsigned i = 0; i < lightsDirect.size(); ++i)
    {
        LightDirect* l = lightsDirect[i];
        Au::Math::Vec3f col = l->Color();
        Au::Math::Vec3f dir = l->Direction();
        glUniform3f(
            glGetUniformLocation(shaderProgram, (std::string("LightDirectRGB[") + std::to_string(i) + "]").c_str()),
            col.x, col.y, col.z
        );
        glUniform3f(
            glGetUniformLocation(shaderProgram, ("LightDirect[" + std::to_string(i) + "]").c_str()),
            dir.x, dir.y, dir.z
        );
    }
    
    for(unsigned i = 0; i < lightsOmni.size(); ++i)
    {
        LightOmni* l = lightsOmni[i];
        Au::Math::Vec3f col = l->Color();
        Au::Math::Vec3f pos = l->GetComponent<Transform>()->WorldPosition();
        glUniform3f(
            glGetUniformLocation(shaderProgram, ("LightOmniRGB[" + std::to_string(i) + "]").c_str()),
            col.x, col.y, col.z
        );
        glUniform3f(
            glGetUniformLocation(shaderProgram, ("LightOmniPos[" + std::to_string(i) + "]").c_str()),
            pos.x, pos.y, pos.z
        );
    }
    
    glUniform3f(
        glGetUniformLocation(shaderProgram, "AmbientColor"),
        0.4f, 0.3f, 0.2f
    );
    
    for(RenderUnit& unit : units)
    {
        unit.skeleton->Update();
        unit.skeleton->Bind(shaderProgram);
        unit.Bind();
        if(unit.diffuse)
            unit.diffuse->Bind(0);
        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "MatrixModel"), 
            1, 
            GL_FALSE, 
            (float*)&unit.transform->GetTransform()
        );
        unit.vao.DrawElements(GL_TRIANGLES);
    }
}