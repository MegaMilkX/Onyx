#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/math.h>

#include "../object.h"

class Mesh;
class GFXScene : public Object::Component
{
friend Mesh;
public:
    void Render(const Au::Math::Mat4f& perspective,
        const Au::Math::Mat4f& transform);

    virtual void OnCreate();
private:
    void AddMesh(Mesh* mesh);
    std::vector<Mesh*> meshes;
};

#endif
