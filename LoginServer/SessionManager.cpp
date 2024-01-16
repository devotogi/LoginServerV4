#include "pch.h"
#include "SessionManager.h"
#include "LoginSession.h"

SessionManager* SessionManager::_instance = nullptr;

void SessionManager::AddSession(int32 sessionId, LoginSession* session)
{
	Lock lock(&_cs);
	_sessions.insert({ sessionId,session });
}

void SessionManager::PopSession(int32 sessionId)
{
	Lock lock(&_cs);
	_sessions.erase(sessionId);
}

void SessionManager::BroadCast(BYTE* dataPtr, int32 dataSize)
{
	Lock lock(&_cs);

	for (auto& session : _sessions)
		session.second->Send(dataPtr, dataSize);
}

void SessionManager::GetSessionId(int32& sessionId)
{
	_sessionId++;
	sessionId = _sessionId;
}

LoginSession* SessionManager::GetSession(int32 sessionId)
{
	Lock lock(&_cs);

	LoginSession* ret = nullptr;

	auto it = _sessions.find(sessionId);

	if (it != _sessions.end())
		ret = it->second;

	return ret;
}
