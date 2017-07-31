
teapot = {};

Init = function()
    Print("Executing scene script");
    
    mesh = SceneObject:CreateObject():Mesh();
    mesh:SetMesh("miku");
    mesh:SetMaterial("material");
    
    mesh = SceneObject:CreateObject():Mesh();
    mesh:SetMesh("cube");
    mesh:SetMaterial("material2");
    mesh:GetObject():Transform():Translate(0.0, 0.0, 4.0);
    
    SceneObject:CreateObject():Script():SetScript("teapot");
    SceneObject:CreateObject():Script():SetScript("sphere");
    SceneObject:CreateObject():Script():SetScript("sphere2");
    SceneObject:CreateObject():Script():SetScript("sphere3");
    
    --mesh = SceneObject:CreateObject():Mesh();
    --mesh:SetMesh("env");
    --mesh:SetMaterial("material");
    
    light = SceneObject:CreateObject():LightOmni();
    light:Color(0.8, 0.4, 1.0);
    
    o = light:GetObject();
    o:Transform():Position(-0.5, 1.7, 0.5);
    
    ld = SceneObject:CreateObject():LightDirect();
    ld:Color(0.8, 0.6, 0.4);
    ld:Direction(0.0, -1.0, 0.0);
    
    ld = SceneObject:CreateObject():LightDirect();
    ld:Color(0.4, 0.3, 0.2);
    ld:Direction(0.0, -0.5, -0.5);
    
    Print("Done");
end

Update = function()
    
end