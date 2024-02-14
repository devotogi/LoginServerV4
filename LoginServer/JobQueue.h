#pragma once
#include "TSQueue.h"
#include "pkt.h"
class LoginJobQueue
{
private:
	TSQueue<LoginObj> _tsqueue;
public:
	static LoginJobQueue* GetInstance()
	{
		static LoginJobQueue jobQueue;
		return &jobQueue;
	}

	void Push(const LoginObj& data);
	void Pop();
	const LoginObj& Front();
	bool SwapQueue();
	bool PopQueueEmpty();
};

