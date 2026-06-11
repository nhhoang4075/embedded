#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

Model::Model() : ImageX(95), modelListener(0), score(0), highScore(0)
{
	modelTickCount = 0;
}

void Model::tick()
{
	modelTickCount++;
}

void Model::update(int16_t x)
{
	ImageX = x;
}

int16_t Model::GetImageX()
{
	return ImageX;
}

void Model::SaveScore(uint16_t s)
{
	score = s;
	if (s > highScore)
	{
		highScore = s;
	}
}

uint16_t Model::GetScore()
{
	return score;
}

uint16_t Model::GetHighScore()
{
	return highScore;
}
