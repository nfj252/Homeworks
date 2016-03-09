#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>

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
	for (int i = 0; i < text.size(); i++) {
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
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

class Player 
{
public:
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	float yTimeMove = 0;
};

class Ball
{
public:
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	float xMovement = 0;
	float yMovement = 0;
};

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Nick's Pong Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	SDL_Event event;
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Matrix projectionMatrix;
	Matrix viewMatrix;
	Matrix leftPlayerModelMatrix;
	Matrix rightPlayerModelMatrix;
	Matrix ballModelMatrix;
	
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

	Player leftPlayer;
	leftPlayer.x = -3.4f;
	leftPlayer.y = 0.0f;
	leftPlayer.width = .25f;
	leftPlayer.height = 1.5f;
	
	Player rightPlayer;
	rightPlayer.x = 3.4f;
	rightPlayer.y = 0.0f;
	rightPlayer.width = .25f;
	rightPlayer.height = 1.5f;

	Ball ball;
	ball.width = 0.3f;

	float leftPlayerModelVerticies[] = {-3.525,-.75, -3.275,-.75, -3.275,.75, -3.525,-.75, -3.275,.75, -3.525,.75};
	float rightPlayerModelVerticies[] = {3.525,-.75, 3.275,-.75, 3.275,.75, 3.525,-.75, 3.275,.75, 3.525,.75 };
	float ballModelVerticies[] = {-.15,-.15, .15,-.15, .15,.15, -.15,-.15, .15,.15, -.15, .15 };
	float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};

	GLuint leftPlayerTexture = LoadTexture("alienBlue_square.png");
	GLuint rightPlayerTexture = LoadTexture("alienPink_square.png");
	GLuint ballTexture = LoadTexture("alienYellow_square.png");

	leftPlayerModelMatrix.identity();
	rightPlayerModelMatrix.identity();
	ballModelMatrix.identity();

	float ticks = (float)SDL_GetTicks() / 1000.0f;  //also in loop
	float lastFrameTicks = ticks - .0005;  
	float elapsed = ticks - lastFrameTicks;

	ball.xMovement = elapsed/2;
	ball.yMovement = elapsed/2;

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	bool done = false;
	while (!done) 
	{
		glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnableVertexAttribArray(program.positionAttribute);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		program.setModelMatrix(leftPlayerModelMatrix);
		glBindTexture(GL_TEXTURE_2D, leftPlayerTexture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftPlayerModelVerticies);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		program.setModelMatrix(rightPlayerModelMatrix);
		glBindTexture(GL_TEXTURE_2D, rightPlayerTexture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightPlayerModelVerticies);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		program.setModelMatrix(ballModelMatrix);
		glBindTexture(GL_TEXTURE_2D, ballTexture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballModelVerticies);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		ball.x += ball.xMovement;
		ball.y += ball.yMovement;

		if (ball.x > rightPlayer.x - rightPlayer.width/2 - ball.width/2 && 
			ball.y < rightPlayer.y + rightPlayer.height/2 - ball.height/2 &&
			ball.y > rightPlayer.y - rightPlayer.height/2 + ball.height/2)
			ball.xMovement = -elapsed/2.0f;
		
		if (ball.x < leftPlayer.x + leftPlayer.width/2 + ball.width/2 &&
			ball.y < leftPlayer.y + leftPlayer.height/2 - ball.height/2 &&
			ball.y > leftPlayer.y - leftPlayer.height/2 + ball.height/2)
			ball.xMovement = elapsed/2.0f;

		if (ball.y > projectionHeight/2 - ball.height/2)
			ball.yMovement = -elapsed/2.0f;

		if (ball.y < -projectionHeight/2 + ball.height/2)
			ball.yMovement = elapsed/2.0f;

		if (ball.x > projectionWidth/2 + ball.width/2 || ball.x < -projectionWidth/2 - ball.width/2)
		{
			ball.xMovement = 0;
			ball.yMovement = 0;

			if (ball.x > projectionWidth / 2 + ball.width / 2)
				leftPlayerModelMatrix.Translate(elapsed, 0,0);
			else
				rightPlayerModelMatrix.Translate(-elapsed, 0,0);
		}

		ballModelMatrix.Translate(ball.xMovement, ball.yMovement, 0.0f);

		if (keys[SDL_SCANCODE_W])
		{
			if (leftPlayer.yTimeMove < 0)
				leftPlayer.yTimeMove = 0;
			leftPlayer.yTimeMove += elapsed / 20;
			leftPlayer.y += leftPlayer.yTimeMove;

			if (leftPlayer.y > projectionHeight / 2 - leftPlayer.height / 2)
				leftPlayer.y = projectionHeight / 2 - leftPlayer.height / 2;
			else
				leftPlayerModelMatrix.Translate(0.0f, leftPlayer.yTimeMove, 0.0f);
		}
		else if (keys[SDL_SCANCODE_S])
		{
			if (leftPlayer.yTimeMove > 0)
				leftPlayer.yTimeMove = 0;
			leftPlayer.yTimeMove -= elapsed / 20;
			leftPlayer.y += leftPlayer.yTimeMove;

			if (leftPlayer.y < -projectionHeight / 2 + leftPlayer.height / 2)
				leftPlayer.y = -projectionHeight / 2 + leftPlayer.height / 2;
			else
				leftPlayerModelMatrix.Translate(0.0f, leftPlayer.yTimeMove, 0.0f);
		}

		if (keys[SDL_SCANCODE_UP])
		{
			if (rightPlayer.yTimeMove < 0)
				rightPlayer.yTimeMove = 0;
			rightPlayer.yTimeMove += elapsed / 20;
			rightPlayer.y += rightPlayer.yTimeMove;

			if (rightPlayer.y > projectionHeight / 2 - rightPlayer.height / 2)
				rightPlayer.y = projectionHeight / 2 - rightPlayer.height / 2;
			else
				rightPlayerModelMatrix.Translate(0.0f, rightPlayer.yTimeMove, 0.0f);
		}
		else if (keys[SDL_SCANCODE_DOWN])
		{
			if (rightPlayer.yTimeMove > 0)
				rightPlayer.yTimeMove = 0;
			rightPlayer.yTimeMove -= elapsed / 20;
			rightPlayer.y += rightPlayer.yTimeMove;

			if (rightPlayer.y < -projectionHeight / 2 + rightPlayer.height / 2)
				rightPlayer.y = -projectionHeight / 2 + rightPlayer.height / 2;
			else
				rightPlayerModelMatrix.Translate(0.0f, rightPlayer.yTimeMove, 0.0f);
		}

		while (SDL_PollEvent(&event))
		{
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
