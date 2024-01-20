#pragma once
#include "pch.h"
#pragma pack (push, 1)
class ServerMovePacket
{
public:
	int32 userSQ;
	int32 palyerSQ;
	int16 movePort;
};
#pragma pack (pop)
