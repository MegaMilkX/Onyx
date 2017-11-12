Material:SetLayer(0, "Diffuse", "add");
Material:SetLayer(100, "Ambient", "multiply");
Material:SetLayer(200, "RimLight", "add");
Material:SetLayer(300, "LightDirectLambert", "add");
Material:SetLayer(400, "LightOmniLambert", "add");

--Material:SetVec3("AmbientColor", 0.0, 0.2, 0.4);
--Material:SetVec3("RimColor", 0.8, 0.4, 0.0);
Material:SetTexture2D("DiffuseTexture", "14924224208470");
Material:SetVec3("AmbientColor", 0.5, 0.5, 0.5);