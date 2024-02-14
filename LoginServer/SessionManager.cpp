#include "pch.h"
#include "SessionManager.h"
#include "LoginSession.h"

SessionManager* SessionManager::_instance = nullptr;

void SessionManager::AddSession(int32 sessionId, LoginSession* session)
{
	LockGuard lock(&_cs);
	_sessions.insert({ sessionId,session });
}

void SessionManager::PopSession(int32 sessionId)
{
	LockGuard lock(&_cs);
	_sessions.erase(sessionId);
}

void SessionManager::BroadCast(BYTE* dataPtr, int32 dataSize)
{
	LockGuard lock(&_cs);

	for (auto& session : _sessions)
		session.second->Send(dataPtr, dataSize);
}

void SessionManager::GetSessionId(int32& sessionId)
{
	InterlockedIncrement(&_sessionId);
	sessionId = _sessionId;
}

LoginSession* SessionManager::GetSession(int32 sessionId)
{
	LockGuard lock(&_cs);

	LoginSession* ret = nullptr;

	auto it = _sessions.find(sessionId);

	if (it != _sessions.end())
		ret = it->second;

	return ret;
}
