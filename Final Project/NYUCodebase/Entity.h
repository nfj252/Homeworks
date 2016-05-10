#include "Matrix.h"
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
using namespace std;

class Entity
{
public:
	float x;
	float y;
	float width;
	float height;
	float xVelocity;
	float yVelocity;
	float xAcceleration;
	float yAcceleration;
	float xFriction;
	float yFriction;
	bool enabled;
	bool bottomContact;
	bool topContact;
	bool leftContact;
	bool rightContact;
	float gravity;
	Matrix matrix;

	Entity();
	~Entity();
	void DynamicUpdateRoutine(float elapsed);
	void StaticUpdateRoutine(float elapsed);
	bool isDirectlyCollidingWith(Entity* other);
	void checkForDirectionalCollision(Entity* other, string direction);
	void handleCollisionWith(Entity* other);
};

