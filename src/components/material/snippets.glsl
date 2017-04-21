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
#vertex PositionModel
    in vec3 Position;
    out vec4 PositionModel = vec4(Position, 1.0);
#vertex NormalModel
    in vec3 Normal;
    uniform mat4 MatrixModel;
    out vec3 NormalModel;
    NormalModel = (MatrixModel * vec4(Normal, 0.0)).xyz;
#vertex FragPosWorld
    in vec3 Position;
    uniform mat4 MatrixModel;
    out vec3 FragPosWorld;
    FragPosWorld = vec3(MatrixModel * vec4(Position, 1.0));
    
#fragment AmbientColor
    uniform vec3 UniformAmbientColor;
    out vec4 AmbientColor = vec4(UniformAmbientColor, 1.0);
#fragment LightDirection
    uniform vec3 LightOmniPos;
    in vec3 FragPosWorld;
    out vec3 LightDirection;
    LightDirection = normalize(LightOmniPos - FragPosWorld);
#fragment LightOmniLambert
    in vec3 NormalModel;
    uniform vec3 LightOmniRGB;
    uniform vec3 LightOmniPos;
    in vec3 FragPosWorld;
    in vec3 LightDirection;
    out vec4 LightOmniLambert;
    float diff = max(dot(NormalModel, LightDirection), 0.0);
    float dist = distance(LightOmniPos, FragPosWorld);
    LightOmniLambert = 
        vec4(
            LightOmniRGB * 
            diff *
            (1.0 / (1.0 + 0.5 * dist + 3.0 * dist * dist)),
            1.0
        );
#fragment RimLight
    uniform mat4 MatrixView;
    in vec3 NormalModel;
    in vec3 FragPosWorld;
    out vec4 RimLight;
    vec3 camPos = inverse(MatrixView)[3].xyz;
    float diff = max(0.3 - dot(NormalModel, normalize(camPos - FragPosWorld)), 0.0);
    RimLight = vec4(diff, diff, diff, 1.0);
        
#fragment DebugRed
    out vec4 DebugRed = vec4(1.0, 0.0, 0.0, 1.0);
        
#generic multiply
    in vec4 first;
    in vec4 second;
    out vec4 result = first * second;
#generic add
    in vec4 first;
    in vec4 second;
    out vec4 result = first + second;

)"
