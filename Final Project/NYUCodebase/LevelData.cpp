#include "LevelData.h"

int* LevelData::numberOfTiles = new int[3];
int* LevelData::numberOfGoals = new int[3];
int* LevelData::numberOfMissiles = new int[3];
int* LevelData::numberOfRaindrops = new int[3];
int* LevelData::missileSpeeds = new int[3];

void LevelData::setUpValues()
{
	numberOfTiles[0] = 40;
	numberOfTiles[1] = 60;
	numberOfTiles[2] = 80;
	
	numberOfGoals[0] = 1;
	numberOfGoals[1] = 2;
	numberOfGoals[2] = 3;

	numberOfMissiles[0] = 16;
	numberOfMissiles[1] = 18;
	numberOfMissiles[2] = 20;

	numberOfRaindrops[0] = 0;
	numberOfRaindrops[1] = 2;
	numberOfRaindrops[2] = 4; 

	missileSpeeds[0] = 2.0f;
	missileSpeeds[1] = 2.25f;
	missileSpeeds[2] = 2.5f;
}
