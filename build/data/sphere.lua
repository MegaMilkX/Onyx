
Init = function()
    SceneObject:Transform():Translate(2.5, 0.0, 1.5);
    
    skel = SceneObject:Skeleton();
    skel:SetData("skin");
    
    SceneObject:Animation():SetAnimData("skin");
    
    mesh = SceneObject:FindObject("Bone003"):Mesh();
    mesh:SetMesh("sphere");
    mesh:SetMaterial("material1");
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end