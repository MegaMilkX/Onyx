R"(#version 450
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
)"