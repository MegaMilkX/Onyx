
Init = function()
    model = SceneObject:Model();
    model.mesh:Set("teapot");
    model.material:Set("material1");
    SceneObject:Transform():Translate(-2.5, 0.0, 1.5);
    SceneObject:SetName("teapot");
end

Update = function()
    SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end