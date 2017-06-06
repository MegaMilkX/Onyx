R"(

#vertex PositionWorld
    in vec3 Position;
    uniform mat4 MatrixModel;
    uniform mat4 MatrixView;
    uniform mat4 MatrixProjection;
    out vec4 PositionWorld;
    
    PositionWorld =  
        MatrixProjection * 
        MatrixView * 
        MatrixModel *
        vec4(Position, 1.0);
        
#vertex SkinWorld
    in vec3 Position;
    uniform mat4 MatrixModel;
    uniform mat4 MatrixView;
    uniform mat4 MatrixProjection;
    in vec4 BoneIndex4;
    in vec4 BoneWeight4;
    uniform mat4 BoneInverseBindTransforms[MAX_BONE_COUNT];
    uniform mat4 BoneTransforms[MAX_BONE_COUNT];
    out vec4 SkinWorld;
    
    int bi0 = int(BoneIndex4.x);
    int bi1 = int(BoneIndex4.y);
    int bi2 = int(BoneIndex4.z);
    int bi3 = int(BoneIndex4.w);
    mat4 mSkin = 
        BoneTransforms[bi0] * BoneInverseBindTransforms[bi0] * BoneWeight4.x +
        BoneTransforms[bi1] * BoneInverseBindTransforms[bi1] * BoneWeight4.y +
        BoneTransforms[bi2] * BoneInverseBindTransforms[bi2] * BoneWeight4.z +
        BoneTransforms[bi3] * BoneInverseBindTransforms[bi3] * BoneWeight4.w;
    
    SkinWorld =  
        MatrixProjection * 
        MatrixView * 
        MatrixModel *
        mSkin *
        vec4(Position, 1.0);
        
#vertex PositionModel
    in vec3 Position;
    out vec4 PositionModel = vec4(Position, 1.0);
#vertex VertexBoneWeight4
    in vec4 BoneWeight4;
    out vec4 VertexBoneWeight4 = BoneWeight4;
#vertex VertexBoneWeight4
    in vec4 BoneIndex4;
    out vec4 VertexBoneIndex4 = BoneIndex4;
#vertex NormalModel
    in vec3 Normal;
    uniform mat4 MatrixModel;
    out vec3 NormalModel;
    NormalModel = normalize((MatrixModel * vec4(Normal, 0.0)).xyz);
#vertex FragPosWorld
    in vec3 Position;
    uniform mat4 MatrixModel;
    out vec3 FragPosWorld;
    FragPosWorld = vec3(MatrixModel * vec4(Position, 1.0));
    
#fragment AmbientColor
    uniform vec3 UniformAmbientColor;
    out vec4 AmbientColor = vec4(UniformAmbientColor, 1.0);
#fragment LightDirection
    uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];
    in vec3 FragPosWorld;
    out vec3 LightDirection[LIGHT_OMNI_COUNT];
    for(int i = 0; i < LIGHT_OMNI_COUNT; i++)
    {
        LightDirection[i] = normalize(LightOmniPos[i] - FragPosWorld);
    }
#fragment LightOmniLambert
    in vec3 NormalModel;
    uniform vec3 LightOmniRGB[LIGHT_OMNI_COUNT];
    uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];
    in vec3 FragPosWorld;
    in vec3 LightDirection[LIGHT_OMNI_COUNT];
    out vec4 LightOmniLambert = vec4(0.0, 0.0, 0.0, 1.0);
    
    for(int i = 0; i < LIGHT_OMNI_COUNT; i++)
    {
        float diff = max(dot(NormalModel, LightDirection[i]), 0.0);
        float dist = distance(LightOmniPos[i], FragPosWorld);
        LightOmniLambert += 
            vec4(
                LightOmniRGB[i] * 
                diff *
                (1.0 / (1.0 + 0.5 * dist + 3.0 * dist * dist)),
                1.0
            );
    }
#fragment LightDirectLambert
    in vec3 NormalModel;
    uniform vec3 LightDirectRGB[LIGHT_DIRECT_COUNT];
    uniform vec3 LightDirect[LIGHT_DIRECT_COUNT];
    out vec4 LightDirectLambert = vec4(0.0, 0.0, 0.0, 1.0);
    
    for(int i = 0; i < LIGHT_DIRECT_COUNT; ++i)
    {
        float diff = max(dot(NormalModel, -LightDirect[i]), 0.0);
        LightDirectLambert += vec4(LightDirectRGB[i] * diff, 1.0);
    }
#fragment RimLight
    uniform vec3 RimColor;
    uniform mat4 MatrixView;
    in vec3 NormalModel;
    in vec3 FragPosWorld;
    out vec4 RimLight;
    vec3 camPos = inverse(MatrixView)[3].xyz;
    float diff = max(0.5 - dot(NormalModel, normalize(camPos - FragPosWorld)), 0.0);
    RimLight = vec4(RimColor * diff, 1.0);
        
#generic DistortFishEye
    in vec2 input;
    out vec2 output;
    
    output = input;
    vec2 v = output.xy / output.w;
    
    float theta = atan(v.y, v.x);
    float radius = length(v);
    radius = pow(radius, 0.8);
    v.x = radius * cos(theta);
    v.y = radius * sin(theta);
    output.xy = v.xy * output.w;

#fragment DebugRed
    out vec4 DebugRed = vec4(1.0, 0.0, 0.0, 1.0);
    
#fragment DebugBoneWeightColor
    in vec4 VertexBoneWeight4;
    out vec4 DebugBoneWeightColor = vec4(VertexBoneWeight4.xyz, 1.0);
    
#fragment DebugBoneIndexColor
    in vec4 VertexBoneIndex4;
    out vec4 DebugBoneIndexColor = vec4(VertexBoneIndex4.xyz, 1.0);
    
#generic multiply
    in vec4 first;
    in vec4 second;
    out vec4 result = first * second;
#generic add
    in vec4 first;
    in vec4 second;
    out vec4 result = first + second;

)"
