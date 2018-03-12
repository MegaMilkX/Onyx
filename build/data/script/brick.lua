
Init = function()
    SceneObject:Transform():Translate(0.0, 0.0, -2.5);
    SceneObject:SetName("brick");
    
    --SceneObject:Animation():SetAnimData("skin");
    
    model = SceneObject:Model();
    model.mesh:Set("brick");
    model.material:Set("matbrick");
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end