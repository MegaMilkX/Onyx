
Init = function()
    SceneObject:Transform():Translate(4.5, 0.0, 1.5);
    
    model = SceneObject:Model();
    model.mesh:Set("skin");
    --model:SetSubMeshIndex(1);
    model.material:Set("material_skin");

    SceneObject:Skeleton():SetData("skin");
    
    SceneObject:Animation():SetAnim("idle", "skin");
    SceneObject:Animation():Play("idle");
    
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end