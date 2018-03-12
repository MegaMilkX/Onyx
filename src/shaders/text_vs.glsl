R"(#version 450
    in vec3 Position;
    in vec3 UVW;

    out vec3 UVFrag;

    uniform mat4 MatrixModel;
    uniform mat4 MatrixView;
    uniform mat4 MatrixProjection;

    void main()
    {
        vec4 PositionScreen;
        vec4 PositionModel;
        
        PositionModel = vec4 ( Position , 1.0 ) ;
        UVFrag = UVW;
        PositionScreen = 
            MatrixProjection *
            MatrixModel *
            (PositionModel);
        gl_Position = PositionScreen ; 
    }
)"