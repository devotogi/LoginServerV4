#include "pch.h"
#include "Session.h"
Session::Session(const SOCKET& socket, const SOCKADDR_IN& sock) : _socket(socket), _sockaddrIn(sock)
{
}

Session::~Session()
{
}

void Session::Send(BYTE* buffer, int32 bufferSize)
{
	// ::send(_socket, (char*)buffer, bufferSize, 0);

	WSABUF WSAbuffer;
	DWORD sendBytes;
	DWORD flag = 0;

	WSAbuffer.buf = reinterpret_cast<CHAR*>(buffer);
	WSAbuffer.len = bufferSize;

	//WSABUF wsaBuf;
	//wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.GetWritePos());
	//wsaBuf.len = _recvBuffer.GetFreeSize();


	// send 여기 처리하라는 뜻              // 비동기 처리 할 곳이 없음
	if (WSASend(_socket, &WSAbuffer, 1, &sendBytes, flag, NULL, NULL) == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();

		if (errorCode == WSAEWOULDBLOCK) 
		{
			wprintf_s(L"WSA Send Block Error\n");
			closesocket(_socket);
		}
		else
		{
			wprintf_s(L"errorCode %d", errorCode);
		}
	}
}

void Session::Recv(int32 numOfBytes)
{
	while (true)
	{
		BYTE* dataPtr = nullptr;
		int32 dataLen = 0;

		if (_recvBuffer.Read(numOfBytes, &dataPtr, dataLen))
		{
			OnRecv(this, dataPtr, dataLen);
			_recvBuffer.AddRecvPos(dataLen);
			numOfBytes = 0;
		}
		else
		{
			break;
		}
	}

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.GetWritePos());
	wsaBuf.len = _recvBuffer.GetFreeSize();
	DWORD flag = 0;
	DWORD newNumOfBytes = 0;

	if (WSARecv(_socket, &wsaBuf, 1, &newNumOfBytes, &flag, &_overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			wprintf_s(L"WSA Recv Error\n");
		}
	}
}

void Session::OnRecv(Session* session, BYTE* dataPtr, int32 dataLen)
{
}

void Session::OnDisconnect()
{

}

void Session::OnConnect()
{
	
}
