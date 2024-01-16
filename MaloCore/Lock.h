#pragma once
class Lock
{
	enum LockType
	{
		NONE,
		MUTEX,
		CS
	};

private:
	LPCRITICAL_SECTION _cs;
	std::mutex* _mutex;
	LockType _lockType = NONE;

public:
	Lock(LPCRITICAL_SECTION cs) : _cs(cs), _lockType(CS)
	{
		EnterCriticalSection(_cs);
	}

	Lock(std::mutex* m) : _mutex(m), _lockType(MUTEX)
	{
		_mutex->lock();
	}

	~Lock()
	{
		switch (_lockType)
		{
		case MUTEX:
			_mutex->unlock();
			break;
		case CS:
			LeaveCriticalSection(_cs);
			break;
		}
	}
};
