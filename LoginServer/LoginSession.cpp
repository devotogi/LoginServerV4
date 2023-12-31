#include "pch.h"
#include "LoginSession.h"
#include "PacketHandler.h"
#include "SessionManager.h"
#include "MyDBConnection.h"
LoginSession::LoginSession(const SOCKET& socket, const SOCKADDR_IN& sock) : Session(socket, sock)
{
}

LoginSession::~LoginSession()
{
}

void LoginSession::OnRecv(Session* session, BYTE* dataPtr, int32 dataLen)
{
	PacketHandler::HandlePacket(static_cast<LoginSession*>(session), dataPtr, dataLen);
}

void LoginSession::OnDisconnect()
{
	SessionManager::GetInstance()->PopSession(_sessionId);

	if (_userSQ == -1)
		return;

	DBConnection* accountCon = AccountDBConnectionPool::GetInstance()->Pop();
	// 로그아웃 시간 DB에 저장
	{
		SQLPrepare(accountCon->GetHSTMT(), (SQLWCHAR*)L"update requies.d_user set LOGOUT_DT = GETDATE() where SQ = ?;", SQL_NTS);
		SQLBindParameter(accountCon->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLINTEGER*)&_userSQ, 0, NULL);
		SQLExecute(accountCon->GetHSTMT());
		SQLFetch(accountCon->GetHSTMT());
		SQLCloseCursor(accountCon->GetHSTMT());
	}
	AccountDBConnectionPool::GetInstance()->Push(accountCon);
}

void LoginSession::OnConnect()
{
	SessionManager::GetInstance()->GetSessionId(_sessionId);
	SessionManager::GetInstance()->AddSession(_sessionId, this);
}
