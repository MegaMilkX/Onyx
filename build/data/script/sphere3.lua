
Init = function()
    SceneObject:Transform():Translate(10.5, 0.0, 1.5);
    
    --skel = SceneObject:Skeleton();
    --skel:SetData("character");
    
    --SceneObject:Animation():SetAnimData("skin");
    
    mesh = SceneObject:SolidMesh();
    mesh:SetMesh("character");
    mesh:SetMaterial("material");
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end