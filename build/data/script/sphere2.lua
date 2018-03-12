
Init = function()
    SceneObject:Transform():Translate(6.5, 0.0, 1.5);
    
    skel = SceneObject:Skeleton();
    skel:SetData("character");
    
    --SceneObject:Animation():SetAnimData("skin");
    
    model = SceneObject:Model();
    model.mesh:Set("character");
    model.material:Set("material");
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end