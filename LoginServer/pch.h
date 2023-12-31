#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug\\MaloCore.lib")
#else 
#pragma comment(lib, "Release\\MaloCore.lib")
#endif
#include "CorePch.h"

int GetRandom0to8();

struct UserInfo 
{
public:
	WCHAR userId[250] = {};
	WCHAR userPw[250] = {};
};