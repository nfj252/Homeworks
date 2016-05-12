#include "LevelData.h"

int* LevelData::numberOfTiles = new int[3];
int* LevelData::numberOfMissiles = new int[3];
int* LevelData::numberOfRaindrops = new int[3];
int* LevelData::missileSpeeds = new int[3];

void LevelData::setUpValues()
{
	numberOfTiles[0] = 40;
	numberOfTiles[1] = 60;
	numberOfTiles[2] = 80;
	
	numberOfMissiles[0] = 12;
	numberOfMissiles[1] = 12;
	numberOfMissiles[2] = 12;

	numberOfRaindrops[0] = 0;
	numberOfRaindrops[1] = 1;
	numberOfRaindrops[2] = 2; 

	missileSpeeds[0] = 2.0f;
	missileSpeeds[1] = 2.25f;
	missileSpeeds[2] = 2.5f;

	//rainDropSpeeds[0] = .75f;
	//rainDropSpeeds[1] = 1.0f;
	//rainDropSpeeds[2] = 1.25f;
}
