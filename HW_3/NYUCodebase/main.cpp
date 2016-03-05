#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
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

class Entity 
{
public:
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	float xTimeMove = 0;
	float yTimeMove = 0;
	bool enabled = true;
};

bool checkForCollision(Entity* a, Entity* b)
{
	if (a->y - a->height / 2 < b->y + b->height / 2 && a->y + a->height / 2 > b->y + b->height / 2 &&
		a->x + a->width / 2 > b->x + b->width / 2 && a->x - a->width / 2 < b->x + b->width / 2)
		return true;
	else
		return false;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Nick's Space Invader Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
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

	//PLAYER OBJECT STUFF
	Entity shipPlayer;
	shipPlayer.y = -1.75f;
	shipPlayer.width = .25f;
	shipPlayer.height = .5f;
	Matrix shipPlayerModelMatrix;

	float shipPlayerModelVerticies[] = {-.125,-2, .125,-2, .125,-1.5, -.125,-2, .125,-1.5, -.125,-1.5};

	//PLAYER PROJECTILE OBJECT STUFF
	vector<Matrix> playerProjectileModelMatrixHolder = {};
	vector<Entity> playerProjectileHolder = {};
	int projectileBulletClipSize = 40;
	int bulletCounter = 0;
	for (int i = 0; i < projectileBulletClipSize; i++)
	{ 
		Entity playerProjectile;
		playerProjectile.enabled = false;
		playerProjectile.x = 0;
		playerProjectile.y = 2.5f;
		playerProjectile.width = .01f;
		playerProjectile.height = .25f;
		playerProjectileHolder.push_back(playerProjectile);

		Matrix playerProjectileMatrix;
		playerProjectileMatrix.setPosition(playerProjectile.x, playerProjectile.y, 0);
		playerProjectileModelMatrixHolder.push_back(playerProjectileMatrix);
		
	}
	float playerProjectileModelVerticies[] = {-.005f,-.125f, .005f,-.125f, .005f, .125f, 
											  -.005f,-.125f, .005f,.125f, -.005f, .125f};

	//ENEMY SHIP OBJECT STUFF
	vector<Matrix> shipEnemyModelMatrixHolder = {};
	vector<Entity> shipEnemyHolder = {};
	int numberOfEnemyRows = 3;
	int numberOfEnemyColumns = 8;
	float upperLeftShipX = -3.0f;
	float upperLeftShipY = 1.7f;
	float offsetX = 0.2f;
	float offsetY = 0.2f;
	float initialShipEnemyModelVerticies[] = {-.1f,-.2f, .1f,-.2f, .1f,.2f, -.1f,-.2f, .1f,.2f, -.1f,.2f};
	for (int i = 0; i < numberOfEnemyRows; i++)
	{
		for (int j = 0; j < numberOfEnemyColumns; j++)
		{
			Entity shipEnemy;
			shipEnemy.width = 0.2f;
			shipEnemy.height = 0.4f;
			shipEnemy.x = upperLeftShipX + j*(shipEnemy.width + offsetX);
			shipEnemy.y = upperLeftShipY - i*(shipEnemy.height + offsetY);
			Matrix shipEnemyModelMatrix;
			shipEnemyModelMatrix.setPosition(shipEnemy.x, shipEnemy.y, 0);

			shipEnemyModelMatrixHolder.push_back(shipEnemyModelMatrix);
			shipEnemyHolder.push_back(shipEnemy);
		}
	}

	/*
	vector<Matrix> enemyProjectileModelMatrixHolder = {};
	vector<Entity> enemyProjectileHolder = {};

	for (int i = 0; i < numberOfEnemyColumns; i++)
	{
		Entity enemyshot;
		enemyshot.width = .0f;
		enemyshot.height = 0.25f;
		enemyshot.x = shipEnemyHolder[i].x;
		enemyshot.y = shipEnemyHolder[i].y;
		Matrix enemyShotMatrix;
		//enemyShotMatrix.setPosition(enemyshot.x, shipEnemy.y, 0);

		shipEnemyModelMatrixHolder.push_back(shipEnemyModelMatrix);
		shipEnemyHolder.push_back(shipEnemy);
	}
	*/


	Matrix titleTextMatrix;
	GLuint font = LoadTexture("font1.png");

	Matrix spriteSheetMatrix;
	GLuint spriteSheetTexture = LoadTexture("spriteSheet.png");
	int index = 3;
	int spriteCountX = 5;
	int spriteCountY = 3;

	float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};

	int gameStatus = 0;

	float ticks = 0.0f;
	float lastFrameTicks = 0.0f;
	float elapsed = 0.0f;
	
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

		if (gameStatus == 0 || gameStatus == 2 || gameStatus == 3)
		{
			program.setModelMatrix(titleTextMatrix);
			if (gameStatus == 0)
				DrawText(&program, font, "Press space to begin", .3f, 0.0f);
			else if (gameStatus == 2)
				DrawText(&program, font, "You Lose", .3f, 0.0f);
			else if (gameStatus == 3)
				DrawText(&program, font, "You Win", .3f, 0.0f);
			titleTextMatrix.setPosition(-2.75f, 0, 0.0f);
		}
		else if (gameStatus == 1)
		{
			program.setModelMatrix(shipPlayerModelMatrix);
			DrawSpriteSheetSprite(&program, spriteSheetTexture, 4, spriteCountX, spriteCountY, shipPlayerModelVerticies);

			for (int i = 0; i < numberOfEnemyRows * numberOfEnemyColumns; i++)
			{
				if (shipEnemyHolder[i].enabled)
					break;
				if (i == numberOfEnemyRows * numberOfEnemyColumns - 1)
					gameStatus = 3;
			}
			
			for (int i = 0; i < numberOfEnemyRows * numberOfEnemyColumns; i++)
			{
				shipEnemyHolder[i].x += shipEnemyHolder[i].xTimeMove;
				if (shipEnemyHolder[i].enabled)
				{
					program.setModelMatrix(shipEnemyModelMatrixHolder[i]);
					DrawSpriteSheetSprite(&program, spriteSheetTexture, 5, spriteCountX, spriteCountY, initialShipEnemyModelVerticies);
					shipEnemyModelMatrixHolder[i].Translate(shipEnemyHolder[i].xTimeMove, 0, 0);
					if (shipEnemyHolder[i].y <= shipPlayer.y || checkForCollision(&shipEnemyHolder[i], &shipPlayer))
					{
						gameStatus = 2;
						break;
					}
				}
			}
			
			if (shipEnemyHolder[0].x <= upperLeftShipX)
			{
				for (int i = 0; i < numberOfEnemyRows * numberOfEnemyColumns; i++)
				{
					shipEnemyHolder[i].xTimeMove = elapsed /2;
					shipEnemyHolder[i].y -= elapsed*200;
					shipEnemyModelMatrixHolder[i].Translate(0, -elapsed*200, 0);
				}
			}
			else if (shipEnemyHolder[numberOfEnemyColumns - 1].x >= -upperLeftShipX)
			{
				for (int i = 0; i < numberOfEnemyRows * numberOfEnemyColumns; i++)
				{
					shipEnemyHolder[i].xTimeMove = -elapsed /2;
					shipEnemyHolder[i].y -= elapsed*200;
					shipEnemyModelMatrixHolder[i].Translate(0, -elapsed*200, 0);
				}
			}
			
			for (int i = 0; i < projectileBulletClipSize; i++)
			{
				playerProjectileHolder[i].yTimeMove = elapsed * 2.0f;

				if (playerProjectileHolder[i].enabled)
				{
					program.setModelMatrix(playerProjectileModelMatrixHolder[i]);
					DrawSpriteSheetSprite(&program, spriteSheetTexture, 6, spriteCountX, spriteCountY, playerProjectileModelVerticies);
					playerProjectileHolder[i].y += playerProjectileHolder[0].yTimeMove;
					playerProjectileModelMatrixHolder[i].Translate(0.0f, playerProjectileHolder[i].yTimeMove, 0.0f);
					
					for (int j = 0; j < numberOfEnemyRows * numberOfEnemyColumns; j++)
					{
						if (checkForCollision(&shipEnemyHolder[j], &playerProjectileHolder[i]))
						{
							if (shipEnemyHolder[j].enabled)
								playerProjectileHolder[i].enabled = false;
							shipEnemyHolder[j].enabled = false;
							break;
						}
					}
				}

				if (playerProjectileHolder[bulletCounter].y >= projectionHeight + playerProjectileHolder[bulletCounter].height / 2)
					playerProjectileHolder[i].enabled = false;
			}
		}

		if (keys[SDL_SCANCODE_RIGHT])
		{
			if (gameStatus == 1)
			{
				if (shipPlayer.xTimeMove < 0)
					shipPlayer.xTimeMove = 0;
				shipPlayer.xTimeMove = elapsed * 2;
				shipPlayer.x += shipPlayer.xTimeMove;

				if (shipPlayer.x > projectionWidth / 2 - shipPlayer.width / 2)
					shipPlayer.x = projectionWidth / 2 - shipPlayer.width / 2;
				else
					shipPlayerModelMatrix.Translate(shipPlayer.xTimeMove, 0.0f, 0.0f);
			}
		}
		else if (keys[SDL_SCANCODE_LEFT])
		{
			if (gameStatus == 1)
			{
				if (shipPlayer.xTimeMove > 0)
					shipPlayer.xTimeMove = 0;
				shipPlayer.xTimeMove = -elapsed * 2;
				shipPlayer.x += shipPlayer.xTimeMove;

				if (shipPlayer.x < -projectionWidth / 2 + shipPlayer.width / 2)
					shipPlayer.x = -projectionWidth / 2 + shipPlayer.width / 2;
				else
					shipPlayerModelMatrix.Translate(shipPlayer.xTimeMove, 0.0f, 0.0f);
			}
		}

		while (SDL_PollEvent(&event))
		{
			if (keys[SDL_SCANCODE_SPACE])
			{
				if (gameStatus == 0)
					gameStatus++;	
			}
			if (keys[SDL_SCANCODE_UP])
			{
				if (gameStatus == 1)
				{
					if (!playerProjectileHolder[bulletCounter].enabled)
					{
						playerProjectileHolder[bulletCounter].x = shipPlayer.x;
						playerProjectileHolder[bulletCounter].y = shipPlayer.y + shipPlayer.height / 2;
						playerProjectileModelMatrixHolder[bulletCounter].setPosition(shipPlayer.x, shipPlayer.y + shipPlayer.height / 2, 0.0f);
						playerProjectileHolder[bulletCounter].enabled = true;
					}
					bulletCounter++;
					if (bulletCounter == projectileBulletClipSize)
						bulletCounter = 0;
				}
			}
			

			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
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
