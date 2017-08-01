
function Init()
    mesh = SceneObject:Mesh();
    mesh:SetMesh("cube");
    mesh:SetMaterial("material2");
    mesh:GetObject():Transform():Translate(0.0, 0.0, 4.0);
end

function Update()
    --SceneObject:Transform():Rotate(0.001, 0.0, 1.0, 0.0);
end

function MouseMove(x, y)
    SceneObject:Transform():Rotate(-x * 0.005, 0.0, 1.0, 0.0);
    SceneObject:Transform():Rotate(-y * 0.005, 1.0, 0.0, 0.0);
end