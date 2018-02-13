
teapot = {};

Init = function()
    Print("Executing scene script");
	
	--floorPlane = SceneObject:CreateObject();
	--floorPlane:PlaneCollider();
	--floorPlane:PlaneCollider():Height(0.0f);
    
    mesh = SceneObject:CreateObject():Mesh();
    mesh:SetMesh("chara6");
    mesh:SetMaterial("material");
    mesh:GetObject():SetName("MIKU");
    
    camera = SceneObject:CreateObject();
    cube = SceneObject:CreateObject();
    --cube:Script():SetScript("cube");
    --camera:Script():SetScript("camera");
    
    --SceneObject:CreateObject():Script():SetScript("teapot");
    --SceneObject:CreateObject():Script():SetScript("sphere");
    --SceneObject:CreateObject():Script():SetScript("brick");
    --SceneObject:CreateObject():Script():SetScript("sphere2");
    
    mesh = SceneObject:CreateObject():Mesh();
    mesh:SetMesh("env");
    mesh:SetMaterial("env");
    --mesh:GetObject():Transform():Scale(0.1);
    mesh:GetObject():MeshCollider():SetMesh("env");
    
    lightHinge = SceneObject:CreateObject();
    lightHinge:Transform():Translate(0.0, 2.0, 0.0);
    
    lightObject = SceneObject:CreateObject();
    lightObject:Transform():AttachTo(lightHinge:Transform());
    lightObject:Transform():Translate(-0.7, 0.0, 0.0);
    lightObject:LightOmni():Color(0.8, 0.4, 1.0);
    
    ld = SceneObject:CreateObject():LightDirect();
    ld:Color(0.4, 0.3, 0.2);
    ld:Direction(0.0, -1.0, 0.0);
    
    ld = SceneObject:CreateObject():LightDirect();
    ld:Color(0.2, 0.15, 0.1);
    ld:Direction(0.0, -0.5, -0.5);
    
    Print("Done");
end

Update = function()
    lightHinge:Transform():Rotate(0.002, 0.0, 1.0, 0.0);
end