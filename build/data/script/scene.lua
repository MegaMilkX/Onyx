
teapot = {};

Init = function()
    Print("Executing scene script");
	
	--floorPlane = SceneObject:CreateObject();
	--floorPlane:PlaneCollider();
	--floorPlane:PlaneCollider():Height(0.0f);
    
    --model = SceneObject:CreateObject():Model();
    --model.mesh:Set("miku2");
    --model.material:Set("material");
    --model:GetObject():SetName("MIKU");
    
    camera = SceneObject:CreateObject();
    cube = SceneObject:CreateObject();
    --cube:Script():SetScript("cube");
    --camera:Script():SetScript("camera");
    
    --SceneObject:CreateObject():Script():SetScript("teapot");
    --SceneObject:CreateObject():Script():SetScript("sphere");
    --SceneObject:CreateObject():Script():SetScript("brick");
    --SceneObject:CreateObject():Script():SetScript("sphere2");
    
    model = SceneObject:CreateObject():Model();
    model.mesh:Set("env2");
    model.material:Set("env");
    --model:GetObject():Transform():Scale(0.1);
    model:GetObject():MeshCollider():SetMesh("env2");
    model:GetObject():SetName("environment");
    
    model = SceneObject:CreateObject():Model();
    model.mesh:Set("naga");
    model.material:Set("material1");
    model:GetObject():MeshCollider():SetMesh("cube");
    model:GetObject():SetName("Secret!");
    --model:GetObject():Transform():Translate(0.0, 0.5, 0.0);
    
    lightHinge = SceneObject:CreateObject();
    lightHinge:Transform():Translate(0.0, 0.5, 0.0);
    
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