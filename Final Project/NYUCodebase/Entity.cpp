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
	gravity = 7.5;
}

Entity::~Entity()
{

}



void Entity::DynamicUpdateRoutine(float elapsed)
{
	xVelocity += xAcceleration * elapsed;
	yVelocity -= gravity * elapsed;
	x += xVelocity * elapsed;
	y += yVelocity * elapsed;
	matrix.Translate(xVelocity * elapsed, yVelocity * elapsed, 0.0f);
}

void Entity::StaticUpdateRoutine(float elapsed)
{

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

void Entity::checkForDirectionalCollision(Entity* other, string direction)
{
	if (direction == "bottom")
	{
		if (y < other->y + other->height * 1.1f && y > other->y && abs(x - other->x) < other->width)
		{
			bottomContact = true;
		}
		else
			bottomContact = false;
	}
	else if (direction == "top")
	{
		if (y > other->y - other->height * 1.1f && y < other->y && abs(x - other->x) < other->width)
		{
			topContact = true;
		}
		else
			topContact = false;
	}
	else if (direction == "left")
	{
		if (x > other->x - other->width * 1.1f && x < other->x && abs(y - other->y) < other->height)
		{
			leftContact = true;
		}
		else
			leftContact = false;
	}
	else if (direction == "right")
	{
		if (x < other->x + other->width * 1.1f && x > other->x && abs(y - other->y) < other->height)
		{
			rightContact = true;
		}
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
			y += YPenetration + .001f;
			matrix.Translate(0, YPenetration + .001f, 0);
		}
		if (y < other->y)
		{
			float YPenetration = abs((y + height / 2) - (other->y - other->height / 2));
			y -= (YPenetration + .001f);
			matrix.Translate(0, -YPenetration - .001f, 0);
		}
	}

	if (abs(y - other->y) < (height + other->height) / 2.5f)
	{
		xVelocity = 0.0f;
		if (x > other->x)
		{
			float XPenetration = abs((x - width / 2) - (other->x + other->width / 2));
			x += XPenetration + .001f;
			matrix.Translate(XPenetration + .001f, 0, 0);
		}
		if (x < other->x)
		{
			float XPenetration = abs((x + width / 2) - (other->x - other->width / 2));
			x -= (XPenetration + .001f);
			matrix.Translate(-XPenetration - .001f, 0, 0);
		}
	}
}


