
Init = function()
    SceneObject:Transform():Translate(0.0, 0.0, -2.5);
    SceneObject:SetName("brick");
    
    --SceneObject:Animation():SetAnimData("skin");
    
    mesh = SceneObject:Mesh();
    mesh:SetMesh("brick");
    mesh:SetMaterial("matbrick");
end

Update = function()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end