R"(#version 450
    in vec2 UVFrag;

    out vec4 fragOut;

    uniform sampler2D DiffuseTexture;

    void main()
    {
        vec4 Diffuse;
         
        Diffuse = texture ( DiffuseTexture , UVFrag ) ; 
        fragOut = Diffuse ; 
    }
)"