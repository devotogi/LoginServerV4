#pragma once
class LoginSession;
class SessionManager
{
private:
	static SessionManager* _instance;
	std::map<int32, LoginSession*> _sessions;
	CRITICAL_SECTION	   _cs;
	uint32				   _sessionId = 0;

public:
	SessionManager()
	{
		InitializeCriticalSection(&_cs);
	}

	~SessionManager()
	{
		DeleteCriticalSection(&_cs);
	}

public:
	void AddSession(int32 sessionId, LoginSession* session);
	void PopSession(int32 sessionId);
	void BroadCast(BYTE* dataPtr, int32 dataSize);
	void GetSessionId(int32& sessionId);
	LoginSession* GetSession(int32 sessionId);

	static SessionManager* GetInstance()
	{
		if (_instance == nullptr)
			_instance = new SessionManager();

		return _instance;
	}
};

