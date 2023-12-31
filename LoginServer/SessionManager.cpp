#include "pch.h"
#include "SessionManager.h"
#include "LoginSession.h"

SessionManager* SessionManager::_instance = nullptr;

void SessionManager::AddSession(int32 sessionId, LoginSession* session)
{
	EnterCriticalSection(&_cs);

	_sessions.insert({ sessionId,session });

	LeaveCriticalSection(&_cs);
}

void SessionManager::PopSession(int32 sessionId)
{
	EnterCriticalSection(&_cs);

	_sessions.erase(sessionId);

	LeaveCriticalSection(&_cs);
}

void SessionManager::BroadCast(BYTE* dataPtr, int32 dataSize)
{
	EnterCriticalSection(&_cs);

	for (auto& session : _sessions)
		session.second->Send(dataPtr, dataSize);

	LeaveCriticalSection(&_cs);
}

void SessionManager::GetSessionId(int32& sessionId)
{
	_sessionId++;
	sessionId = _sessionId;
}

LoginSession* SessionManager::GetSession(int32 sessionId)
{
	LoginSession* ret = nullptr;

	EnterCriticalSection(&_cs);
	auto it = _sessions.find(sessionId);

	if (it != _sessions.end())
		ret = it->second;

	LeaveCriticalSection(&_cs);

	return ret;
}
