#include "Entity.h"

Entity::Entity()
{
	x = 0;
	y = 0;
	width = .5;
	height = .5;
	xVelocity = 0;
    yVelocity = 0;
	xAcceleration = 0;
    yAcceleration = 0;
	xFriction = 5;
	yFriction = 1;
	enabled = true;
	bottomContact = false;
	topContact = false;
	leftContact = false;
	rightContact = false;
	gravity = -10;
}

Entity::~Entity()
{

}


void Entity::DynamicUpdateRoutine(float elapsed)
{
	xVelocity += xAcceleration * elapsed;
	if (xVelocity >= 2.5f)
		xVelocity = 2.5f;
	else if (xVelocity <= -2.5f)
		xVelocity = -2.5f;
	yVelocity += gravity * elapsed;
	x += xVelocity * elapsed;
	y += yVelocity * elapsed;
	matrix.Translate(xVelocity * elapsed, yVelocity * elapsed, 0.0f);
}

void Entity::MissleUpdateRoutine(float elapsed)
{
	if (x <= -15)
	{
		x = xSpawnPoint;
		y = .5f + -(rand() % 8) / 2.0f;
		matrix.setPosition(x,y,0);
	}
	x += xVelocity * elapsed;
	matrix.Translate(xVelocity * elapsed, yVelocity * elapsed, 0.0f);
}

void Entity::RainDropUpdateRoutine(float elapsed)
{
	if (y <= -5)
	{
		x = xSpawnPoint + (rand() % 26) / 2.0f;
		y = ySpawnPoint;
		yVelocity = 0;
		matrix.setPosition(x, y, 0);
	}
	if ((leftContact || rightContact) && bottomContact)
		yVelocity = 3;
	yVelocity += gravity * elapsed;
	y += yVelocity * elapsed;
	x += xVelocity * elapsed;
	matrix.Translate(xVelocity * elapsed, yVelocity * elapsed, 0.0f);
}

bool Entity::isDirectlyCollidingWith(Entity* other)
{
	if (y - height / 2 < other->y + other->height / 2 && y + height / 2 > other->y - other->height / 2 &&
	x + width / 2 > other->x - other->width / 2 && x - width / 2 < other->x + other->width / 2)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Entity::checkForDirectionalCollision(Entity* other, string direction, float offset)
{
	if (direction == "bottom")
	{
		if (y - height / 2 < other->y + other->height / 2 + offset && y > other->y && abs(x - other->x) < (other->width + width) / 2 - offset)
			bottomContact = true;
		else
			bottomContact = false;
	}
	else if (direction == "top")
	{
		if (y + height / 2 > other->y - other->height / 2 - offset && y < other->y && abs(x - other->x) < (other->width + width) / 2 - offset)
			topContact = true;
		else
			topContact = false;
	}
	else if (direction == "left")
	{
		if (x - width / 2 < other->x + other->width / 2 + offset && x > other->x && abs(y - other->y) < (other->height + height) / 2 - offset)
			leftContact = true;
		else
			leftContact = false;
	}
	else if (direction == "right")
	{
		if (x + width / 2 > other->x - other->width / 2 - offset && x < other->x && abs(y - other->y) < (other->height + height) / 2 - offset)
			rightContact = true;
		else
			rightContact = false;
	}
}

void Entity::handleCollisionWith(Entity* other)
{
	if (abs(x - other->x) < (width + other->width) / 2.5f)
	{
		yVelocity = 0.0f;
		if (y > other->y)
		{
			float YPenetration = abs((y - height / 2) - (other->y + other->height / 2));
			y += YPenetration + .0001f;
			matrix.Translate(0, YPenetration + .0001f, 0);
		}
		if (y < other->y)
		{
			float YPenetration = abs((y + height / 2) - (other->y - other->height / 2));
			y -= (YPenetration + .0001f);
			matrix.Translate(0, -YPenetration - .0001f, 0);
		}
	}

	if (abs(y - other->y) < (height + other->height) / 2.5f)
	{
		xVelocity = 0.0f;
		if (x > other->x)
		{
			float XPenetration = abs((x - width / 2) - (other->x + other->width / 2));
			x += XPenetration + .0001f;
			matrix.Translate(XPenetration + .0001f, 0, 0);
		}
		if (x < other->x)
		{
			float XPenetration = abs((x + width / 2) - (other->x - other->width / 2));
			x -= (XPenetration + .0001f);
			matrix.Translate(-XPenetration - .0001f, 0, 0);
		}
	}
}


