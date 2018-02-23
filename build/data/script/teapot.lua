
Init = function()
    mesh = SceneObject:SolidMesh();
    mesh:SetMesh("teapot");
    mesh:SetMaterial("material1");
    SceneObject:Transform():Translate(-2.5, 0.0, 1.5);
    SceneObject:SetName("teapot");
end

Update = function()
    SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end