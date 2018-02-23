
function Init()
    mesh = SceneObject:SolidMesh();
    mesh:SetMesh("character");
    mesh:SetMaterial("material2");
    mesh:GetObject():Transform():Translate(0.0, 5.0, 4.0);
    
    SceneObject:RigidBody():SetAngularFactor(Vec3(0,0,0));
    
    SceneObject:SetName("MyCube");
end

keys = {false,false,false,false}

function Update()
    cam = SceneObject:Root():Renderer():GetCurrentCamera();

    vec = Vec3(0.0, 0.0, 0.0);
    act = false;
    
    if keys[0] == true then
        vec = Vec3Add(vec, cam:GetObject():Transform():Forward());
        act = true;
    end
    if keys[1] == true then
        vec = Vec3Add(vec, cam:GetObject():Transform():Left());
        act = true;
    end
    if keys[2] == true then
        vec = Vec3Add(vec, cam:GetObject():Transform():Back());
        act = true;
    end
    if keys[3] == true then
        vec = Vec3Add(vec, cam:GetObject():Transform():Right());
        act = true;
    end
    
    vec = Vec3(vec.x, 0.0, vec.z);
    vec = Vec3Normalize(vec);
    vec = Vec3MultScalar(vec, 5);

    if act == true then
        pos = SceneObject:Transform():GetPosition();
        SceneObject:RigidBody():LookAt(Vec3Add(vec, pos), pos);
        SceneObject:RigidBody():SetLinearVelocity(Vec3(vec.x, vec.y, vec.z));
    end
end

W = 87;
A = 65;
S = 83;
D = 68;

function KeyDown(key)
    if key == W then
        keys[0] = true;
    elseif key == A then
        keys[1] = true;
    elseif key == S then
        keys[2] = true;
    elseif key == D then
        keys[3] = true;
    end
end

function KeyUp(key)
    if key == W then
        keys[0] = false;
    elseif key == A then
        keys[1] = false;
    elseif key == S then
        keys[2] = false;
    elseif key == D then
        keys[3] = false;
    end
end

function MouseMove(x, y)
    SceneObject:Transform():Rotate(-x * 0.005, 0.0, 1.0, 0.0);
    SceneObject:Transform():Rotate(-y * 0.005, 1.0, 0.0, 0.0);
end