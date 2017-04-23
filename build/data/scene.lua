
teapot = {};

Init = function()
    Print("Executing scene script");
    
    light = SceneObject:CreateObject():LightOmni();
    light:Color(0.8, 0.4, 1.0);
    Print("Calling GetObject()");
    o = light:GetObject();Print("GetObject() done");
    o:Transform():Position(-0.5, 1.7, 0.5);
    
    ld = SceneObject:CreateObject():LightDirect();
    ld:Color(0.8, 0.6, 0.4);
    ld:Direction(0.0, -1.0, 0.0);
    
    mesh = SceneObject:CreateObject():Mesh();
    mesh:SetMesh("miku");
    mesh:SetMaterial("material");
    
    teapot = SceneObject:CreateObject():Mesh();
    teapot:SetMesh("teapot");
    teapot:SetMaterial("material1");
    teapot:GetObject():Transform():Translate(-2.5, 0.0, 1.5);
    
    Print("Done");
end

Update = function()
    teapot:GetObject():Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end