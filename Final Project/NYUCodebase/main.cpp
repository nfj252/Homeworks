#ifdef _WINDOWS
	#include <GL/glew.h>
	#include <SDL_mixer.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include "FunctionRepository.h"
#include "LevelDataHolder.h"
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <vector>
#include <string>
using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Save Yo Friend", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	SDL_Event event;
	int screenWidth = 640;
	int screenHeight = 360;
	glViewport(0, 0, screenWidth, screenHeight);   //axis starts from lower left (x,y,w,h)
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Matrix projectionMatrix;
	Matrix viewMatrix;

	float projectionWidth = 7.1f;
	float projectionHeight = 4.0f;
	projectionMatrix.setOrthoProjection(-projectionWidth/2, projectionWidth/2, 
										-projectionHeight/2, projectionHeight/2, 
										-1.0f, 1.0f); //(l,r,b,t,n,f) n/f doesnt do anything in ortho

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Music *musicPlaying = Mix_LoadMUS("muffins.mp3");
	Mix_Music *musicWin = Mix_LoadMUS("wombo.mp3");
	Mix_Music *musicLose = Mix_LoadMUS("wombo.mp3");
	Mix_Music *musicStart = Mix_LoadMUS("Chandelier.mp3");
	Mix_PlayMusic(musicStart, -1);

	Entity player;
	vector<Entity> staticEntities;
	Entity goal;
	float playerModelVerticies[] = { -.25f, -.25f, .25f, -.25f, .25f, .25f, -.25f, -.25f, .25f, .25f, -.25f, .25f };
	float blockModelVerticies[] = { -.25f, -.25f, .25f, -.25f, .25f, .25f, -.25f, -.25f, .25f, .25f, -.25f, .25f };
	float goalModelVerticies[] = { -.25f, -.25f, .25f, -.25f, .25f, .25f, -.25f, -.25f, .25f, .25f, -.25f, .25f };
	float backgroundModelVerticies[] = { -20, -10, 20, -10, 20, 10, -20, -10, 20, 10, -20, 10 };
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	Matrix titleTextMatrix;
	Matrix messageTextMatrix;
	Matrix inGameBGTextureMatrix;
	Matrix titleBGTextureMatrix;
	GLuint font = LoadTexture("font1.png");

	GLuint dynamicSpriteSheetTexture = LoadTexture("master-spritesheet.png"); //30 x 30
	GLuint staticSpriteSheetTexture = LoadTexture("master-tileset.png"); //10 x 5
	GLuint backgroundTexture = LoadTexture("bg_shroom.png");
	
	int gameStatus = 0;
	int level = 1;
	float ticks = 0.0f;
	float lastFrameTicks = 0.0f;
	float elapsed = 0.0f;

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	bool done = false;
	while (!done) 
	{
		glClearColor(0.5f, 0.3f, 0.3f, 255.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);
		glEnableVertexAttribArray(program.positionAttribute);
		glEnableVertexAttribArray(program.texCoordAttribute);
		
		ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		if (gameStatus == 0 || gameStatus == 1 || gameStatus == 2)
		{
			viewMatrix.identity();
			titleTextMatrix.setPosition(-2.75f, .5, 0.0f);
			messageTextMatrix.setPosition(-2.75f, -.5, 0.0f);
			program.setModelMatrix(titleTextMatrix);

			if (gameStatus == 0)
			{
				DrawText(&program, font, "'Meet your friend at the end!'", .2f, 0.0f);
				program.setModelMatrix(messageTextMatrix);
				DrawText(&program, font, "Press enter to begin", .2f, 0.0f);
			}
			else if (gameStatus == 1 || gameStatus == 2)
			{
				if (gameStatus == 1)	
					DrawText(&program, font, "You Lose! Press Enter to replay", .2f, 0.0f);
				else if (gameStatus == 2)
					DrawText(&program, font, "You Win! Press Enter to Proceed", .2f, 0.0f);
				program.setModelMatrix(messageTextMatrix);
				DrawText(&program, font, "or press Q to quit", .2f, 0.0f);
			}
		}
		else if (gameStatus == 3)
		{
			program.setModelMatrix(inGameBGTextureMatrix);
			DrawSpriteSheetSprite(&program, backgroundTexture, 0, 1, 1, backgroundModelVerticies);

			program.setModelMatrix(goal.matrix);
			DrawSpriteSheetSprite(&program, dynamicSpriteSheetTexture, 79, 30, 30, goalModelVerticies);

			program.setModelMatrix(player.matrix);
			player.DynamicUpdateRoutine(elapsed);
			if (player.bottomContact)
				DrawSpriteSheetSprite(&program, dynamicSpriteSheetTexture, 20, 30, 30, playerModelVerticies);
			else
				DrawSpriteSheetSprite(&program, dynamicSpriteSheetTexture, 29, 30, 30, playerModelVerticies);
			
			viewMatrix.setPosition(-player.x, -player.y, 0);

			for (unsigned i = 0; i < staticEntities.size(); i++)
			{
				program.setModelMatrix(staticEntities[i].matrix);
				DrawSpriteSheetSprite(&program, staticSpriteSheetTexture, 20, 10, 5, blockModelVerticies);
				if (player.isDirectlyCollidingWith(&staticEntities[i]))
					player.handleCollisionWith(&staticEntities[i]);
			}

			for (unsigned i = 0; i < staticEntities.size(); i++)
			{
				player.checkForDirectionalCollision(&staticEntities[i], "bottom");
				if (player.bottomContact)
					break;
			}
			
			if (player.y + player.height / 2 <= -4)
			{
				gameStatus = 1;
				Mix_PlayMusic(musicLose, -1);
			}

			if (player.isDirectlyCollidingWith(&goal))
			{
				gameStatus = 2;
				Mix_PlayMusic(musicWin, -1);
			}
			
			if (keys[SDL_SCANCODE_RIGHT])
			{
				playerModelVerticies[0] = -.25f;
				playerModelVerticies[2] = .25f;
				playerModelVerticies[4] = .25f;
				playerModelVerticies[6] = -.25f;
				playerModelVerticies[8] = .25f;
				playerModelVerticies[10] = -.25f;

				if (player.xVelocity < 0)
					player.xVelocity = 0;
				player.xAcceleration = 2.5f;
			}
			else if (keys[SDL_SCANCODE_LEFT])
			{
				playerModelVerticies[0] = .25f;
				playerModelVerticies[2] = -.25f;
				playerModelVerticies[4] = -.25f;
				playerModelVerticies[6] = .25f;
				playerModelVerticies[8] = -.25f;
				playerModelVerticies[10] = .25f;

				if (player.xVelocity > 0)
					player.xVelocity = 0;
				player.xAcceleration = -2.5f;
			}
			else if (keys == SDL_GetKeyboardState(NULL))
			{
				player.xAcceleration = 0;
				player.xVelocity = lerp(player.xVelocity, 0.0f, elapsed * player.xFriction);
			}
		}

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
			{
				done = true;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if (gameStatus == 0)  //start
				{
					if (keys[SDL_SCANCODE_RETURN])
					{
						Mix_PlayMusic(musicPlaying, -1);
						initialGameSetup(level, &staticEntities, &player, &goal);
						Mix_PlayMusic(musicPlaying, -1);
						gameStatus = 3;
					}
					else if (keys[SDL_SCANCODE_Q] || event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
					{
						done = true;
					}
				}
				else if (gameStatus == 1)  //lose
				{
					if (keys[SDL_SCANCODE_RETURN])
					{
						resetGame(&staticEntities, &player, &goal);
						Mix_PlayMusic(musicPlaying, -1);
						gameStatus = 3;
					}
					else if (keys[SDL_SCANCODE_Q] || event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
					{
						done = true;
					}
				}
				else if (gameStatus == 2)  //win
				{
					if (keys[SDL_SCANCODE_RETURN])
					{
						level++;
						initialGameSetup(level, &staticEntities, &player, &goal);
						Mix_PlayMusic(musicPlaying, -1);
						gameStatus = 3;
					}
					else if (keys[SDL_SCANCODE_Q] || event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
					{
						done = true;
					}
				}
				else if (gameStatus == 3)
				{
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
					{
						if (player.bottomContact)
						{
							Mix_PlayChannel(-1, Mix_LoadWAV("jumpSound.wav"), 0);
							player.yVelocity = 4.0f;
						}
					}
				}
			}
		}
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}