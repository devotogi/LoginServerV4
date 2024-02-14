#pragma once
template<typename T>
class TSQueue
{
private:
	SpinLock _cs;
	std::queue<T> _pushQueue;
	std::queue<T> _popQueue;

	std::queue<T>* _writePointer;
	std::queue<T>* _readPointer;

public:
	void Push(const T& data);
	void Pop();
	const T& Front();
	bool SwapQueue();
	bool PopQueueEmpty();
};

template<typename T>
inline void TSQueue<T>::Push(const T& data)
{
	LockGuard lock(&_cs);
	_pushQueue.push(data);
}

template<typename T>
inline void TSQueue<T>::Pop()
{
	_popQueue.pop();
}

template<typename T>
inline const T& TSQueue<T>::Front()
{
	return _popQueue.front();
}

template<typename T>
inline bool TSQueue<T>::SwapQueue()
{
	bool ret = true;
	bool emptyPushQueue = _pushQueue.empty();

	if (emptyPushQueue)
		return false;
	{
		LockGuard lock(&_cs);
		std::swap(_pushQueue, _popQueue);
	}
	return ret;
}

template<typename T>
inline bool TSQueue<T>::PopQueueEmpty()
{
	return _popQueue.empty();
}
