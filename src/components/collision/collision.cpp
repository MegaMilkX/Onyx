#include "collision.h"

#include "collider.h"

void Collision::AddCollider(Collider* col)
{
	RemoveCollider(col);
	world->addCollisionObject(col->collisionObject);
}

void Collision::RemoveCollider(Collider* col)
{
	world->removeCollisionObject(col->collisionObject);
}