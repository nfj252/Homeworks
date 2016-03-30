#ifdef _WINDOWS
#include <GL/glew.h>
#include <SDL_mixer.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
SDL_Window* displayWindow;


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

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
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

class Entity 
{
public:
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	float xVelocity = 0;  
	float yVelocity = 0;  
	float xAcceleration = 0;
	float yAcceleration = 0;
	float xFriction = 1;
	float yFriction = 1;
	bool enabled = true;
	bool bottomContact = false;
	bool topContact = false;
	bool leftContact = false;
	bool rightContact = false;
	Matrix matrix;

	void resetCollisionFlags()
	{
		bottomContact = false;
		topContact = false;
		leftContact = false;
		rightContact = false;
	}

	bool checkForCollisionWith(Entity* other)
	{
		if (y - height / 2 < other->y + other->height / 2 && y + height / 2 > other->y - other->height / 2 &&
			x + width / 2 > other->x - other->width / 2 && x - width / 2 < other->x + other->width / 2)
		{
			return true;
		}
		else
		{
			resetCollisionFlags();
			return false;
		}
	}

	void handleCollisionWith(Entity* other)
	{
		if (abs(x - other->x) < other->width/2)
		{
			yVelocity = 0.0f;
			if (y > other->y)
			{
				float YPenetration = abs((y - height / 2) - (other->y + other->height / 2));
				y += YPenetration + .001f;
				matrix.Translate(0, YPenetration + .001f, 0);
				bottomContact = true;
			}
			if (y < other->y)
			{
				float YPenetration = abs((y + height / 2) - (other->y - other->height / 2));
				y -= (YPenetration + .001f);
				matrix.Translate(0, -YPenetration - .001f, 0);
				topContact = true;
			}
		}

		if (abs(y - other->y) < other->height / 2)
		{
			xVelocity = 0.0f;
			if (x > other->x)
			{
				float XPenetration = abs((x - width / 2) - (other->x + other->width / 2));
				x += XPenetration + .001f;
				matrix.Translate(XPenetration + .001f, 0, 0);
				leftContact = true;
			}
			if (x < other->x)
			{
				float XPenetration = abs((x + width / 2) - (other->x - other->width / 2));
				x -= (XPenetration + .001f);
				matrix.Translate(-XPenetration - .001f, 0, 0);
				rightContact = true;
			}
		}
	}
};

void resetGame(vector<Entity>* staticContainer, Entity* thePlayer, Entity* theGoal)
{
	staticContainer->clear();
	for (float i = -5; i < 5; i += .5)
	{
		Entity floor;
		floor.x = i * 2;
		floor.y = -(rand() % 6) / 2.0f;
		floor.width = .5f;
		floor.height = .5f;
		floor.matrix.setPosition(floor.x, floor.y, 0);
		staticContainer->push_back(floor);
	}

	thePlayer->x = -10.0f;
	thePlayer->y = 3.0f;
	thePlayer->width = .25f;
	thePlayer->height = .5f;
	thePlayer->xFriction = .1f;
	thePlayer->yFriction = .1f;
	thePlayer->matrix.setPosition(thePlayer->x, thePlayer->y, 0);

	theGoal->x = (*staticContainer)[staticContainer->size() - 1].x;
	theGoal->y = (*staticContainer)[staticContainer->size() - 1].y + (*staticContainer)[staticContainer->size() - 1].height;
	theGoal->width = .5f;
	theGoal->height = .5f;
	theGoal->matrix.setPosition(theGoal->x, theGoal->y, 0);
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Nick's Platformer Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	SDL_Event event;
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Matrix projectionMatrix;
	Matrix viewMatrix;
	
	int screenWidth = 640;
	int screenHeight = 360;
	glViewport(0, 0, screenWidth, screenHeight);   //axis starts from lower left (x,y,w,h)
	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float projectionWidth = 7.1f;
	float projectionHeight = 4.0f;
	projectionMatrix.setOrthoProjection(-projectionWidth/2, projectionWidth/2, 
										-projectionHeight/2, projectionHeight/2, 
										-1.0f, 1.0f); //(l,r,b,t,n,f) n/f doesnt do anything in ortho

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Music *musicStart = Mix_LoadMUS("muffins.mp3");
	Mix_Music *musicFinish = Mix_LoadMUS("wombo.mp3");
	Mix_PlayMusic(musicStart, -1);

	Entity player;
	float playerModelVerticies[] = { -.125, -.25, .125, -.25, .125, .25, -.125, -.25, .125, .25, -.125, .25 };
	vector<Entity> staticEntities;
	float floorModelVerticies[] = { -.25, -.25, .25, -.25, .25, .25, -.25, -.25, .25, .25, -.25, .25 };
	Entity goal;
	srand(time(NULL));
	resetGame(&staticEntities, &player, &goal);

	Matrix titleTextMatrix;
	Matrix messageTextMatrix;
	GLuint font = LoadTexture("font1.png");

	Matrix spriteSheetMatrix;
	GLuint spriteSheetTexture = LoadTexture("spriteSheet.png");
	int index = 3;
	int spriteCountX = 5;
	int spriteCountY = 3;

	float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
	int gameStatus = 0;
	float world_gravity = 0.5f;
	float ticks = 0.0f;
	float lastFrameTicks = 0.0f;
	float elapsed = 0.0f;
	float fixedElapsed = 0.0f;

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	bool done = false;
	while (!done) 
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnableVertexAttribArray(program.positionAttribute);
		glEnableVertexAttribArray(program.texCoordAttribute);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		fixedElapsed = elapsed;

		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS)
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;

		while (fixedElapsed >= FIXED_TIMESTEP)
		{
			fixedElapsed -= FIXED_TIMESTEP;
			//neeed help here
		}
		
		if (gameStatus == 0 || gameStatus == 2 || gameStatus == 3)
		{
			viewMatrix.identity();
			titleTextMatrix.setPosition(-2.75f, .5, 0.0f);
			messageTextMatrix.setPosition(-2.75f, -.5, 0.0f);
			program.setModelMatrix(titleTextMatrix);

			if (gameStatus == 0)
			{
				DrawText(&program, font, "'Meet your friend at the end!'", .2f, 0.0f);
				program.setModelMatrix(messageTextMatrix);
				DrawText(&program, font, "Press enter to begin", .3f, 0.0f);
			}
			else if (gameStatus == 2 || gameStatus == 3)
			{
				if (gameStatus == 2)	
					DrawText(&program, font, "You Lose! Press Enter to replay", .2f, 0.0f);
				else if (gameStatus == 3)
					DrawText(&program, font, "You Win! Press Enter to replay", .2f, 0.0f);
				program.setModelMatrix(messageTextMatrix);
				DrawText(&program, font, "or press Q to quit", .2f, 0.0f);

				while (SDL_PollEvent(&event))
				{
					if (keys[SDL_SCANCODE_RETURN])
					{
						resetGame(&staticEntities, &player, &goal);
						gameStatus = 1;
					}
					else if (keys[SDL_SCANCODE_Q])
					{
						done = true;
					}
					else if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
					{
						done = true;
					}
				}
			}
		}
		else if (gameStatus == 1)
		{
			program.setModelMatrix(player.matrix);
			DrawSpriteSheetSprite(&program, spriteSheetTexture, 4, spriteCountX, spriteCountY, playerModelVerticies);
			player.xVelocity += player.xAcceleration * FIXED_TIMESTEP;
			player.yVelocity -= world_gravity * elapsed;
			player.x += player.xVelocity * FIXED_TIMESTEP;
			player.y += player.yVelocity * FIXED_TIMESTEP;
			player.matrix.Translate(player.xVelocity * FIXED_TIMESTEP, player.yVelocity * FIXED_TIMESTEP, 0.0f);
			viewMatrix.setPosition(-player.x, -player.y, 0);

			for (unsigned i = 0; i < staticEntities.size(); i++)
			{
				program.setModelMatrix(staticEntities[i].matrix);
				DrawSpriteSheetSprite(&program, spriteSheetTexture, 5, spriteCountX, spriteCountY, floorModelVerticies);
				if (player.checkForCollisionWith(&staticEntities[i]))
					player.handleCollisionWith(&staticEntities[i]);
			}

			program.setModelMatrix(goal.matrix);
			DrawSpriteSheetSprite(&program, spriteSheetTexture, 7, spriteCountX, spriteCountY, floorModelVerticies);

			/*
			//Debug Log
			program.setModelMatrix(titleTextMatrix);
			DrawText(&program, font, to_string(player.y), .3f, 0.0f);
			if (player.bottomContact)
				DrawText(&program, font, "true", .3f, 0.0f);
			else
				DrawText(&program, font, "false", .3f, 0.0f);
			*/

			if (player.y + player.height/2 <= -4)
				gameStatus = 2;
			if (player.checkForCollisionWith(&goal))
				gameStatus = 3;
			
			if (keys[SDL_SCANCODE_RIGHT])
			{
				if (player.xVelocity < 0)
					player.xVelocity = 0;
				player.xAcceleration = elapsed * 5;
			}
			else if (keys[SDL_SCANCODE_LEFT])
			{
				if (player.xVelocity > 0)
					player.xVelocity = 0;
				player.xAcceleration = -elapsed * 5;
			}
			else if (keys == SDL_GetKeyboardState(NULL))
			{
				player.xAcceleration = 0;
				player.xVelocity = lerp(player.xVelocity, 0.0f, FIXED_TIMESTEP * player.xFriction);
			}
		}

		while (SDL_PollEvent(&event))
		{
			if (keys[SDL_SCANCODE_RETURN])
			{
				if (gameStatus == 0)
					gameStatus++;	
			}
			else if (keys[SDL_SCANCODE_SPACE])
			{
				//if (player.bottomContact)
				if (player.yVelocity >= -0.005f && player.yVelocity <= 0.005f)
					player.yVelocity = 0.175f;
			}
			else if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
			{
				done = true;
			}
		}

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
