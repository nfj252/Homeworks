#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include "Entity.h"
#include "LevelData.h"
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
using namespace std;

GLuint LoadTexture(const char *image_path)
{
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing)
{
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	for (UINT i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
}

void DrawSpriteSheetSprite(ShaderProgram *program, int spriteTexture, int index, int spriteCountX, int spriteCountY,
	float matrixVerticies[])
{
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0f / (float)spriteCountX;
	float spriteHeight = 1.0f / (float)spriteCountY;

	float textureVerticies[] = { u, v + spriteHeight, u + spriteWidth, v + spriteHeight, u + spriteWidth, v,
		u, v + spriteHeight, u + spriteWidth, v, u, v };

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, matrixVerticies);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, textureVerticies);
	glBindTexture(GL_TEXTURE_2D, spriteTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

float lerp(float v0, float v1, float t)
{
	return (1.0f - t)*v0 + t*v1;
}

void initialGameSetup(unsigned level, vector<Entity>* staticContainer, vector<Entity>* missileContainer, vector<Entity>* rainDropContainer , Entity* thePlayer, Entity* theGoal)
{
	LevelData::setUpValues();
	int seed = static_cast<int>(time(0));
	srand(seed);
	staticContainer->clear();
	missileContainer->clear();
	rainDropContainer->clear();

	for (float i = -10; i < -10 + LevelData::numberOfTiles[level] / 2; i += .5f)
	{
		Entity floor;
		floor.width = .5f;
		floor.height = .5f;
		floor.x = i;
		if (i == -10)
			floor.y = 0;
		else
			floor.y = -(rand() % 7) / 2.0f;
		floor.matrix.setPosition(floor.x, floor.y, 0);
		staticContainer->push_back(floor);
	}

	for (float i = 0; i < LevelData::numberOfMissiles[level]; i++)
	{
		Entity missile;
		missile.width = .5f;
		missile.height = .15f;
		missile.x = (rand() % 30);
		missile.y = .5f + -(rand() % 9) / 2.0f;
		missile.xVelocity = -LevelData::missileSpeeds[level];
		missile.matrix.setPosition(missile.x, missile.y, 0);
		missileContainer->push_back(missile);
	}

	for (float i = 0; i < LevelData::numberOfRaindrops[level]; i++)
	{
		Entity raindrop;
		raindrop.width = .3f;
		raindrop.height = .4f;
		raindrop.x = (rand() % 26) / 2.0f;
		raindrop.y = 3 + (rand() % 8) / 2.0f;
		raindrop.matrix.setPosition(raindrop.x, raindrop.y, 0);
		rainDropContainer->push_back(raindrop);
	}

	thePlayer->x = (*staticContainer)[0].x;
	thePlayer->y = (*staticContainer)[0].y + (*staticContainer)[0].height / 2 + thePlayer->height / 2;
	thePlayer->matrix.setPosition(thePlayer->x, thePlayer->y, 0);

	theGoal->x = (*staticContainer)[staticContainer->size() - 1].x;
	theGoal->y = (*staticContainer)[staticContainer->size() - 1].y + (*staticContainer)[staticContainer->size() - 1].height / 2 + theGoal->height / 2;
	theGoal->matrix.setPosition(theGoal->x, theGoal->y, 0);
}
