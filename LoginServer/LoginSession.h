#pragma once
#include "Session.h"

class LoginSession : public Session
{
private:
	WCHAR _username[256] = {};
	int32 _userSQ = -1;
	int32 _lastServerPort = 0;
	Vector3 _lastPos = { -1,-1,-1 };

public:
	static Session* MakeGameSession(const SOCKET& socket, const SOCKADDR_IN& sockAddr) { return new LoginSession(socket, sockAddr); }

public:
	LoginSession(const SOCKET& socket, const SOCKADDR_IN& sock);
	virtual ~LoginSession();

	virtual void OnRecv(Session* session, BYTE* dataPtr, int32 dataLen);
	virtual void OnDisconnect();
	virtual void OnConnect();
};

