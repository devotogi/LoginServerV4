#include "pch.h"
#include "LoginSession.h"
#include "ThreadManager.h"
#include "IOCPCore.h"
#include "ServerService.h"
#include "DBConnectionPool.h"
#include "MyDBConnection.h"
unsigned int _stdcall Dispatch(void* Args)
{
	ServerService* service = reinterpret_cast<ServerService*>(Args);
	while (true)
		service->GetIOCPCore()->Dispatch();
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
		ThreadManager::GetInstance()->Launch(Dispatch, &service);

	service.Start();
	return 0;
}