#include "LevelData.h"

int* LevelData::numberOfTiles = new int[3];
int* LevelData::numberOfMissiles = new int[3];

void LevelData::setUpValues()
{
	numberOfTiles[0] = 40;
	numberOfTiles[1] = 60;
	numberOfTiles[2] = 80;
	
	numberOfMissiles[0] = 0;
	numberOfMissiles[1] = 20;
	numberOfMissiles[2] = 40;
}
