
Init = function()
    SceneObject:Transform():Translate(6.5, 0.0, 1.5);
    
    skel = SceneObject:Skeleton();
    skel:SetData("skin");
    
    --SceneObject:Animation():SetAnimData("skin");
    
    mesh = SceneObject:Mesh();
    mesh:SetMesh("skin");
    mesh:SetMaterial("material");
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end