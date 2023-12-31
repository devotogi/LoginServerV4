#pragma once
#include "DBConnectionPool.h"

class PlayerDBConnectionPool : public DBConnectionPool
{
public:
	static PlayerDBConnectionPool* GetInstance()
	{
		static PlayerDBConnectionPool pools;
		return &pools;
	}
};

class AccountDBConnectionPool : public DBConnectionPool
{
public:
	static AccountDBConnectionPool* GetInstance()
	{
		static AccountDBConnectionPool pools;
		return &pools;
	}
};

