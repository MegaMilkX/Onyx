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
#fragment LightDirection
    uniform vec3 LightPosition;
    in vec3 FragPosWorld;
    out vec3 LightDirection;
    LightDirection = normalize(LightPosition - FragPosWorld);
#fragment LightOmniLambert
    in vec3 NormalModel;
    uniform vec3 LightRGB;
    uniform vec3 LightPosition;
    in vec3 FragPosWorld;
    in vec3 LightDirection;
    out vec3 LightOmniLambert;
    float diff = max(dot(NormalModel, LightDirection), 0.0);
    float dist = distance(LightPosition, FragPosWorld);
    LightOmniLambert = 
        LightRGB * 
        diff *
        (1.0 / (1.0 + 0.5 * dist + 3.0 * dist * dist));
#generic multiply
    in vec4 first;
    in vec4 second;
    out vec4 result = first * second;
#generic add
    in vec4 first;
    in vec4 second;
    out vec4 result = first + second;

)"
