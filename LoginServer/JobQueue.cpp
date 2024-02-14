#include "pch.h"
#include "JobQueue.h"

void LoginJobQueue::Push(const LoginObj& data)
{
	_tsqueue.Push(data);
}

void LoginJobQueue::Pop()
{
	_tsqueue.Pop();
}

const LoginObj& LoginJobQueue::Front()
{
	return _tsqueue.Front();
}

bool LoginJobQueue::SwapQueue()
{
	return _tsqueue.SwapQueue();
}

bool LoginJobQueue::PopQueueEmpty()
{
	return _tsqueue.PopQueueEmpty();
}
