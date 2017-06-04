
Init = function()
    SceneObject:Transform():Translate(2.5, 0.0, 1.5);
    
    mesh = SceneObject:Mesh();
    mesh:SetMesh("skin");
    mesh:SetMaterial("material_skin");

    SceneObject:Skeleton():SetData("skin");
    
    SceneObject:Animation():SetAnimData("skin");
    
    
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end