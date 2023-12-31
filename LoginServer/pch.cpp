#include "pch.h"

int GetRandom0to8()
{
	thread_local std::mt19937 generator(std::random_device{}());

	std::uniform_int_distribution<int> distribution(0, 9);

	return distribution(generator);
}