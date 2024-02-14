#include "pch.h"
#include "LoginSession.h"
#include "ThreadManager.h"
#include "IOCPCore.h"
#include "ServerService.h"
#include "DBConnectionPool.h"
#include "MyDBConnection.h"
#include "JobQueue.h"
#include "pkt.h"
#include "BufferWriter.h"
#include "SessionManager.h"
unsigned int _stdcall DispatchProc(void* Args)
{
	ServerService* service = reinterpret_cast<ServerService*>(Args);
	while (true)
		service->GetIOCPCore()->Dispatch();

	return 0;
}

unsigned int _stdcall AcceptProc(void* Args)
{
	ServerService* service = reinterpret_cast<ServerService*>(Args);
	service->Start();

	return 0;
}

void LoginProc() 
{
	LoginJobQueue* loginJobQueue = LoginJobQueue::GetInstance();
	bool work = loginJobQueue->SwapQueue();

	if (work == false)
		return;

	try 
	{
		while (loginJobQueue->PopQueueEmpty() == false)
		{
			LoginObj loginObj = loginJobQueue->Front();
			loginJobQueue->Pop();

			LoginSession* session = SessionManager::GetInstance()->GetSession(loginObj.sessionId);

			if (session == nullptr)
				continue;

			UserInfo userInfo;
			SQLINTEGER  SQ;
			DBConnection* con = AccountDBConnectionPool::GetInstance()->Pop();
			{
				WCHAR buffer[256] = {};
				int32 userIdLen = wcslen(loginObj.userId);
				::memcpy(buffer, loginObj.userId, sizeof(WCHAR) * userIdLen);

				SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"select top 1 USER_ID , USER_PW, SQ from requies.d_user where USER_ID = ? ;", SQL_NTS);
				SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(buffer), 0, (SQLWCHAR*)buffer, sizeof(buffer), NULL);

				WCHAR userId2;
				SQLLEN len = 0;
				SQLBindCol(con->GetHSTMT(), 1, SQL_WCHAR, (SQLWCHAR*)&userInfo.userId, sizeof(userInfo.userId), &len);
				SQLBindCol(con->GetHSTMT(), 2, SQL_WCHAR, (SQLWCHAR*)&userInfo.userPw, sizeof(userInfo.userPw), &len);
				SQLBindCol(con->GetHSTMT(), 3, SQL_INTEGER, &SQ, sizeof(SQ), &len);

				SQLExecute(con->GetHSTMT());
				SQLFetch(con->GetHSTMT());
				SQLCloseCursor(con->GetHSTMT());
			}

			int32 userIdSize = wcslen(userInfo.userId);
			int32 compare = wcscmp(loginObj.userPw, userInfo.userPw);

			BYTE sendBuffer[1000];
			BufferWriter bw(sendBuffer);
			PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
			int32 canLogin = 9999;

			if (compare != 0)
			{
				// 로그인 실패
				canLogin = 10000;
			}

			// 로그인 성공
			bw.Write(canLogin);
			// 유저 SQ
			bw.Write((int32)SQ);
			bw.Write(ServerPort::FIELD_SERVER);

			pktHeader->_type = PacketProtocol::S2C_LOGIN;
			pktHeader->_pktSize = bw.GetWriterSize();

			session->Send(sendBuffer, bw.GetWriterSize());

			/*
				로그인 로그 남기기
			*/
			{
				WCHAR buffer[256] = {};
				int32 userIdLen = wcslen(loginObj.userId);
				::memcpy(buffer, loginObj.userId, sizeof(WCHAR) * userIdLen);
				SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"update requies.d_user set LOGIN_DT = GETDATE() where USER_ID = ?;", SQL_NTS);
				SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(buffer), 0, (SQLWCHAR*)buffer, sizeof(buffer), NULL);
				SQLExecute(con->GetHSTMT());
				SQLFetch(con->GetHSTMT());
				SQLCloseCursor(con->GetHSTMT());
			}

			AccountDBConnectionPool::GetInstance()->Push(con);
		}
	}
	catch (const char* errorMessage) {
		std::cerr << "Error: " << errorMessage << std::endl;
	}
}

int main() 
{
	PlayerDBConnectionPool::GetInstance()->Init(L"PLAYER", L"sa", L"root", 5);
	AccountDBConnectionPool::GetInstance()->Init(L"MSSQL", L"sa", L"root", 5);

	std::cout << "LoginServer" << std::endl;
	const char* ip = "58.236.130.58";
	ServerService service(ip, 30003, LoginSession::MakeGameSession);

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	int32 threadCount = sysInfo.dwNumberOfProcessors * 2;

	for (int i = 0; i < threadCount; i++)
		ThreadManager::GetInstance()->Launch(DispatchProc, &service);

	ThreadManager::GetInstance()->Launch(AcceptProc, &service);

	while (true)
	{
		LoginProc();
		Sleep(0);
	}

	return 0;
}