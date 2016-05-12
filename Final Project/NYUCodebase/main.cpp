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
	displayWindow = SDL_CreateWindow("Save Yo Friends", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
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

	Entity player;
	vector<Entity> missileEntities;
	vector<Entity> rainDropEntities;
	vector<Entity> staticEntities;
	vector<Entity> goalEntities;
	float playerModelVerticies[] = { -.2f, -.2f, .2f, -.2f, .2f, .2f, -.2f, -.2f, .2f, .2f, -.2f, .2f };
	float blockModelVerticies[] = { -.25f, -.25f, .25f, -.25f, .25f, .25f, -.25f, -.25f, .25f, .25f, -.25f, .25f };
	float missleModelVerticies[] = { -.25f, -.075f, .25f, -.075f, .25f, .075f, -.25f, -.075f, .25f, .075f, -.25f, .075f };
	float rainDropsModelVerticies[] = { -.15f, -.2f, .15f, -.2f, .15f, .2f, -.15f, -.2f, .15f, .2f, -.15f, .2f };
	float goalModelVerticies[] = { -.2f, -.2f, .2f, -.2f, .2f, .2f, -.2f, -.2f, .2f, .2f, -.2f, .2f };
	float backgroundModelVerticies[] = { -30, -15, 30, -15, 30, 15, -30, -15, 30, 15, -30, 15 };
	float PanelModelVerticies[] = { -15, -8, 15, -8, 15, 8, -15, -8, 15, 8, -15, 8 };
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Music *musicPlaying = Mix_LoadMUS("Merry Go.mp3");
	Mix_Music *musicWin = Mix_LoadMUS("Winning Moment.mp3");
	Mix_Music *musicLose = Mix_LoadMUS("Angel.mp3");
	Mix_Music *musicStart = Mix_LoadMUS("Morning.mp3");
	Mix_PlayMusic(musicStart, -1);

	Matrix titleTextMatrix;
	Matrix messageTextMatrix1;
	Matrix messageTextMatrix2;
	Matrix inGameBGTextureMatrix;
	Matrix titleBGTextureMatrix;
	GLuint font = LoadTexture("font1.png");

	GLuint dynamicSpriteSheetTexture = LoadTexture("master-spritesheet.png"); //30 x 30
	GLuint staticSpriteSheetTexture = LoadTexture("master-tileset.png"); //10 x 5
	GLuint backgroundTextureInGame = LoadTexture("bg_shroom.png");
	GLuint backgroundTextureIntro = LoadTexture("intro_background.png");
	GLuint backgroundTextureEnd = LoadTexture("bg_shroom.png");
	
	int gameStatus = 0;
	int level = 0;
	int numberOfGoals = 0;
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

			if (gameStatus == 0)
			{
				program.setModelMatrix(inGameBGTextureMatrix);
				DrawSpriteSheetSprite(&program, backgroundTextureIntro, 0, 1, 1, PanelModelVerticies);
				titleTextMatrix.setPosition(-3.15f, .5, 0.0f);
				program.setModelMatrix(titleTextMatrix);
				DrawText(&program, font, "Meet Your Friends", .4f, 0.0f);
				messageTextMatrix1.setPosition(-2.85f, -.6, 0.0f);
				program.setModelMatrix(messageTextMatrix1);
				DrawText(&program, font, "Press enter to begin", .3f, 0.0f);
				
			}
			else if (gameStatus == 1 || gameStatus == 2)
			{
				program.setModelMatrix(inGameBGTextureMatrix);
				DrawSpriteSheetSprite(&program, backgroundTextureEnd, 0, 1, 1, PanelModelVerticies);
				titleTextMatrix.setPosition(-1.5f, .5, 0.0f);
				program.setModelMatrix(titleTextMatrix);
				if (gameStatus == 1)
				{
					DrawText(&program, font, "You Died", .4f, 0.0f);
					messageTextMatrix1.setPosition(-2.2f, -.6, 0.0f);
					program.setModelMatrix(messageTextMatrix1);
					DrawText(&program, font, "Press 'Enter' to Replay", .2f, 0.0f);
				}
				else if (gameStatus == 2)
				{
					DrawText(&program, font, "You Win!", .4f, 0.0f);
					messageTextMatrix1.setPosition(-2.25f, -.6, 0.0f);
					program.setModelMatrix(messageTextMatrix1);
					DrawText(&program, font, "Press Enter To Continue", .2f, 0.0f);
				}
				messageTextMatrix2.setPosition(-1.6f, -1, 0.0f);
				program.setModelMatrix(messageTextMatrix2);
				DrawText(&program, font, "Press 'Q' to Quit", .2f, 0.0f);
			}
		}
		else if (gameStatus == 3)
		{
			viewMatrix.setPosition(-player.x, -player.y, 0);
			program.setModelMatrix(inGameBGTextureMatrix);
			DrawSpriteSheetSprite(&program, backgroundTextureInGame, 0, 1, 1, backgroundModelVerticies);

			program.setModelMatrix(player.matrix);
			player.DynamicUpdateRoutine(elapsed);
			if (player.bottomContact)
				DrawSpriteSheetSprite(&program, dynamicSpriteSheetTexture, 20, 30, 30, playerModelVerticies);
			else
				DrawSpriteSheetSprite(&program, dynamicSpriteSheetTexture, 29, 30, 30, playerModelVerticies);
			
			for (unsigned i = 0; i < goalEntities.size(); i++)
			{
				if (goalEntities[i].enabled)
				{
					program.setModelMatrix(goalEntities[i].matrix);
					goalEntities[i].StaticUpdateRoutine(elapsed);
					DrawSpriteSheetSprite(&program, dynamicSpriteSheetTexture, 79, 30, 30, goalModelVerticies);
				
					if (player.isDirectlyCollidingWith(&goalEntities[i]))
					{
						goalEntities[i].enabled = false;
						numberOfGoals--;
						Mix_PlayChannel(-1, Mix_LoadWAV("scoreSound.wav"), 0);
						if (numberOfGoals == 0)
						{
							Mix_PlayMusic(musicWin, -1);
							gameStatus = 2;
						}
					}
				}
			}

			for (unsigned i = 0; i < staticEntities.size(); i++)
			{
				program.setModelMatrix(staticEntities[i].matrix);
				staticEntities[i].StaticUpdateRoutine(elapsed);
				DrawSpriteSheetSprite(&program, staticSpriteSheetTexture, 20, 10, 5, blockModelVerticies);
				if (player.isDirectlyCollidingWith(&staticEntities[i]))
					player.handleCollisionWith(&staticEntities[i]);
			}

			for (unsigned i = 0; i < missileEntities.size(); i++)
			{
				program.setModelMatrix(missileEntities[i].matrix);
				missileEntities[i].xSpawnPoint = player.x + projectionWidth/2 + missileEntities[i].width/2;
				missileEntities[i].MissleUpdateRoutine(elapsed);
				DrawSpriteSheetSprite(&program, dynamicSpriteSheetTexture, 338, 30, 30, missleModelVerticies);
				if (player.isDirectlyCollidingWith(&missileEntities[i]))
					player.handleCollisionWith(&missileEntities[i]);
				for (unsigned j = 0; j < rainDropEntities.size(); j++)
				{
					if (rainDropEntities[j].isDirectlyCollidingWith(&missileEntities[i]))
						rainDropEntities[j].handleCollisionWith(&missileEntities[i]);
				}
			}

			for (unsigned i = 0; i < rainDropEntities.size(); i++)
			{
				program.setModelMatrix(rainDropEntities[i].matrix);
				rainDropEntities[i].ySpawnPoint = player.y + projectionHeight / 2 + rainDropEntities[i].height / 2;
				rainDropEntities[i].xSpawnPoint = player.x - projectionWidth / 2;
				rainDropEntities[i].RainDropUpdateRoutine(elapsed);
				DrawSpriteSheetSprite(&program, dynamicSpriteSheetTexture, 446, 30, 30, rainDropsModelVerticies);

				if (rainDropEntities[i].x > player.x + .4f)
					rainDropEntities[i].xVelocity = -.75f;
				else if (rainDropEntities[i].x < player.x - .4f)
					rainDropEntities[i].xVelocity = .75f;

				if (player.isDirectlyCollidingWith(&rainDropEntities[i]))
				{
					gameStatus = 1;
					Mix_PlayChannel(-1, Mix_LoadWAV("loseSound.wav"), 0);
					Mix_PlayMusic(musicLose, -1);
				}

				for (unsigned j = 0; j < staticEntities.size(); j++)
				{
					if (rainDropEntities[i].isDirectlyCollidingWith(&staticEntities[j]))
						rainDropEntities[i].handleCollisionWith(&staticEntities[j]);
				}
			}

			titleTextMatrix.setPosition(player.x - 2.8f, player.y + 1.8f, 0.0f);
			program.setModelMatrix(titleTextMatrix);
			DrawText(&program, font, "Friends left to meet: ", .25f, 0.0f);
			messageTextMatrix1.setPosition(player.x + 2.8f, player.y + 1.8f, 0.0f);
			program.setModelMatrix(messageTextMatrix1);
			ostringstream ss;
			ss << numberOfGoals;
			DrawText(&program, font, ss.str(), .25f, 0.0f);

			for (unsigned i = 0; i < staticEntities.size(); i++)
			{
				player.checkForDirectionalCollision(&staticEntities[i], "bottom", .001f);
				if (player.bottomContact)
					break;
			}
			
			if (!player.bottomContact)
			{
				for (unsigned i = 0; i < missileEntities.size(); i++)
				{
					player.checkForDirectionalCollision(&missileEntities[i], "bottom", .001f);
					if (player.bottomContact)
					{
						if (player.xAcceleration == 0)
						player.xVelocity = missileEntities[i].xVelocity;
						break;
					}
				}
			}

			for (unsigned i = 0; i < rainDropEntities.size(); i++)
			{
				for (unsigned j = 0; j < staticEntities.size(); j++)
				{
					rainDropEntities[i].checkForDirectionalCollision(&staticEntities[j], "bottom", .001f);
					if (rainDropEntities[i].bottomContact)
						break;
				}
			}

			for (unsigned i = 0; i < rainDropEntities.size(); i++)
			{
				for (unsigned j = 0; j < staticEntities.size(); j++)
				{
					rainDropEntities[i].checkForDirectionalCollision(&staticEntities[j], "left", .001f);
					if (rainDropEntities[i].leftContact)
						break;
				}
			}

			for (unsigned i = 0; i < rainDropEntities.size(); i++)
			{
				for (unsigned j = 0; j < staticEntities.size(); j++)
				{
					rainDropEntities[i].checkForDirectionalCollision(&staticEntities[j], "right", .001f);
					if (rainDropEntities[i].rightContact)
						break;
				}
			}
	
			if (player.y + player.height / 2 <= -6)
			{
				gameStatus = 1;
				Mix_PlayChannel(-1, Mix_LoadWAV("loseSound.wav"), 0);
				Mix_PlayMusic(musicLose, -1);
			}

			if (keys[SDL_SCANCODE_RIGHT])
			{
				playerModelVerticies[0] = -.2f;
				playerModelVerticies[2] = .2f;
				playerModelVerticies[4] = .2f;
				playerModelVerticies[6] = -.2f;
				playerModelVerticies[8] = .2f;
				playerModelVerticies[10] = -.2f;

				if (player.xVelocity < 0)
					player.xVelocity = 0;
				player.xAcceleration = 3.5f;
			}
			else if (keys[SDL_SCANCODE_LEFT])
			{
				playerModelVerticies[0] = .2f;
				playerModelVerticies[2] = -.2f;
				playerModelVerticies[4] = -.2f;
				playerModelVerticies[6] = .2f;
				playerModelVerticies[8] = -.2f;
				playerModelVerticies[10] = .2f;

				if (player.xVelocity > 0)
					player.xVelocity = 0;
				player.xAcceleration = -3.5f;
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
				done = true;
			else if (event.type == SDL_KEYDOWN)
			{
				if (gameStatus == 0 || gameStatus == 1 || gameStatus == 2)  //1 = loss, 2 = win
				{
					if (keys[SDL_SCANCODE_RETURN])
					{
					    if(gameStatus == 2)
						{ 
							level++;
							if (level > 2)
								level = 0;
						}
						initialGameSetup(level, &player, &goalEntities, &staticEntities, &missileEntities, &rainDropEntities);
						numberOfGoals = LevelData::numberOfGoals[level];
						Mix_PlayMusic(musicPlaying, -1);
						gameStatus = 3;
					}
					else if (keys[SDL_SCANCODE_Q])
						done = true;
				}
				else if (gameStatus == 3)
				{
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
					{
						if (player.bottomContact)
						{
							Mix_PlayChannel(-1, Mix_LoadWAV("jumpSound.wav"), 0);
							player.yVelocity = 4.15f;
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