
Init = function()
    mesh = SceneObject:Mesh();
    mesh:SetMesh("teapot");
    mesh:SetMaterial("material1");
    SceneObject:Transform():Translate(-2.5, 0.0, 1.5);
end

Update = function()
    SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end