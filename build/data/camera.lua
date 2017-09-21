
function Init()
    camera = SceneObject:CreateObject();
    camera:Camera():Perspective(1.6, 16.0/9.0, 0.01, 1000.0);
    camera:Transform():Translate(0.0, 0.0, 1.5);
    camera:Transform():AttachTo(SceneObject:Transform());
    
    cube = SceneObject:Root():FindObject("character");
end

function Update()
    vec = cube:Transform():GetPosition();
    vec = Vec3(vec.x, vec.y + 1.5, vec.z);
    SceneObject:Transform():PositionVec(vec);
end

function MouseMove(x, y)
    SceneObject:Transform():Rotate(-x * 0.005, 0.0, 1.0, 0.0);
    SceneObject:Transform():RotateVec(-y * 0.005, SceneObject:Transform():Right());
end