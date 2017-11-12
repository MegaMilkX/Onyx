
Init = function()
    SceneObject:Transform():Translate(4.5, 0.0, 1.5);
    
    mesh = SceneObject:Mesh();
    mesh:SetMesh("skin");
    --mesh:SetSubMeshIndex(1);
    mesh:SetMaterial("material_skin");

    SceneObject:Skeleton():SetData("skin");
    
    SceneObject:Animation():SetAnim("idle", "skin");
    SceneObject:Animation():Play("idle");
    
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end