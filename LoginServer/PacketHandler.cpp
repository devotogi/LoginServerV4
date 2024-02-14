#include "pch.h"
#include "PacketHandler.h"
#include "BufferWriter.h"
#include "LoginSession.h"
#include "BufferReader.h"
#include "pkt.h"
#include "MyDBConnection.h"
#include "PacketHeader.h"
#include "JobQueue.h"
void PacketHandler::HandlePacket(LoginSession* session, BYTE* packet, int32 packetSize)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(packet);
	BYTE* dataPtr = packet + sizeof(PacketHeader);
	int32 dataSize = packetSize - sizeof(PacketHeader);

	switch (header->_type) 
	{
	case PacketProtocol::C2S_LOGIN: // 플레이어 동기화
		HandlePacket_C2S_LOGIN(session, dataPtr, dataSize);
		break;

	case PacketProtocol::C2S_CREATECHARACTER:
		HandlePacket_C2S_CREATECHARACTER(session, dataPtr, dataSize);
		break;

	case PacketProtocol::C2S_CHARACTERLIST:
		HandlePacket_C2S_CHARACTERLIST(session, dataPtr, dataSize);
		break;

	case PacketProtocol::C2S_DELETECHARACTER:
		HandlePacket_C2S_DELETECHARACTER(session, dataPtr, dataSize);
		break;

	case PacketProtocol::C2S_GAMEPLAY:
		HandlePacket_C2S_GAMEPLAY(session, dataPtr, dataSize);
		break;

	case PacketProtocol::C2S_SERVER_MOVE:
		HandlePacket_C2S_SERVER_MOVE(session, dataPtr, dataSize);
		break;
	}
}

void PacketHandler::HandlePacket_C2S_LOGIN(LoginSession* session, BYTE* packet, int32 packetSize)
{
	int32 userIdLen;
	int32 userPwLen;

	BufferReader br(packet);
	LoginObj loginObj;
	loginObj.sessionId = session->GetSessionID();
	br.Read(loginObj.userIdLen);
	br.ReadWString(loginObj.userId, loginObj.userIdLen);
	br.Read(loginObj.userPwLen);
	br.ReadWString(loginObj.userPw, loginObj.userPwLen);
	LoginJobQueue::GetInstance()->Push(loginObj);

	//// Login 처리
	//UserInfo userInfo;
	//SQLINTEGER  SQ;
	//DBConnection* con = AccountDBConnectionPool::GetInstance()->Pop();
	//{
	//	WCHAR buffer[256] = {};
	//	int32 userIdLen = wcslen(userId);
	//	::memcpy(buffer, userId, sizeof(WCHAR) * userIdLen);

	//	SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"select top 1 USER_ID , USER_PW, SQ from requies.d_user where USER_ID = ? ;", SQL_NTS);
	//	SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(buffer), 0, (SQLWCHAR*)buffer, sizeof(buffer), NULL);

	//	WCHAR userId2;
	//	SQLLEN len = 0;
	//	SQLBindCol(con->GetHSTMT(), 1, SQL_WCHAR, (SQLWCHAR*)&userInfo.userId, sizeof(userInfo.userId), &len);
	//	SQLBindCol(con->GetHSTMT(), 2, SQL_WCHAR, (SQLWCHAR*)&userInfo.userPw, sizeof(userInfo.userPw), &len);
	//	SQLBindCol(con->GetHSTMT(), 3, SQL_INTEGER, &SQ, sizeof(SQ), &len);

	//	SQLExecute(con->GetHSTMT());
	//	SQLFetch(con->GetHSTMT());
	//	SQLCloseCursor(con->GetHSTMT());
	//}

	//int32 userIdSize = wcslen(userInfo.userId);
	//int32 compare = wcscmp(userPw, userInfo.userPw);

	//BYTE sendBuffer[1000];
	//BufferWriter bw(sendBuffer);
	//PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
	//int32 canLogin = 9999;

	//if (compare != 0)
	//{
	//	// 로그인 실패
	//	canLogin = 10000;
	//}

	//// 로그인 성공
	//bw.Write(canLogin);
	//// 유저 SQ
	//bw.Write((int32)SQ);
	//bw.Write(ServerPort::FIELD_SERVER);

	//pktHeader->_type = PacketProtocol::S2C_LOGIN;
	//pktHeader->_pktSize = bw.GetWriterSize();

	//session->Send(sendBuffer, bw.GetWriterSize());

	///*
	//	로그인 로그 남기기
	//*/
	//{
	//	WCHAR buffer[256] = {};
	//	int32 userIdLen = wcslen(userId);
	//	::memcpy(buffer, userId, sizeof(WCHAR) * userIdLen);
	//	SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"update requies.d_user set LOGIN_DT = GETDATE() where USER_ID = ?;", SQL_NTS);
	//	SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(buffer), 0, (SQLWCHAR*)buffer, sizeof(buffer), NULL);
	//	SQLExecute(con->GetHSTMT());
	//	SQLFetch(con->GetHSTMT());
	//	SQLCloseCursor(con->GetHSTMT());
	//}

	//AccountDBConnectionPool::GetInstance()->Push(con);
}

void PacketHandler::HandlePacket_C2S_CREATECHARACTER(LoginSession* session, BYTE* packet, int32 packetSize)
{
	int32 userSQ;
	int32 playerType;
	int32 userNameLen;
	WCHAR userName[256] = {0};
	BufferReader br(packet);
	br.Read(userSQ);
	br.Read(playerType);
	br.Read(userNameLen);
	br.ReadWString(userName, userNameLen);
	DBConnection* con = PlayerDBConnectionPool::GetInstance()->Pop();
	int32 cnt;
	int32 playerTotalCnt;
	SQLRETURN retcode;

	// DB에서 중복되었는지 조회
	{
		WCHAR buffer[256] = {};
		::memcpy(buffer, userName, userNameLen);

		SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"select count(PLAYER_NAME), (select count(PLAYER_NAME) from player.d_player where USER_SQ = ?) from player.d_player where PLAYER_NAME = ?;", SQL_NTS);
		
		SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLINTEGER*)&userSQ, 0, NULL);
		SQLBindParameter(con->GetHSTMT(), 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(buffer), 0, (SQLWCHAR*)buffer, sizeof(buffer), NULL);

		SQLLEN len = 0;
		SQLBindCol(con->GetHSTMT(), 1, SQL_INTEGER, &cnt, sizeof(cnt), &len);
		SQLBindCol(con->GetHSTMT(), 2, SQL_INTEGER, &playerTotalCnt, sizeof(playerTotalCnt), &len);

		retcode = SQLExecute(con->GetHSTMT());
		SQLFetch(con->GetHSTMT());
		SQLCloseCursor(con->GetHSTMT());

		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) 
		{
			// 에러 핸들링
			SQLSMALLINT recNumber = 1;
			SQLWCHAR sqlstate[255];
			SQLINTEGER nativeError;
			SQLWCHAR messageText[256];
			SQLSMALLINT textLength;

			while (SQLGetDiagRec(SQL_HANDLE_STMT, con->GetHSTMT(), recNumber, sqlstate, &nativeError, messageText, sizeof(messageText), &textLength) != SQL_NO_DATA) {
				std::cerr << "SQLSTATE: " << sqlstate << std::endl;
				std::cerr << "Native Error: " << nativeError << std::endl;
				std::cerr << "Message: " << messageText << std::endl;

				recNumber++;
			}
		}
	}

	int32 code = 0; // 0 정상처리, 1 이미 중복된 닉네임
	if (cnt != 0) 
	{
		BYTE sendBuffer[1000] = {};
		BufferWriter bw(sendBuffer);
		PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
		pktHeader->_type = PacketProtocol::S2C_CREATECHARACTER;
		code = 1;
		bw.Write(code);
		pktHeader->_pktSize = bw.GetWriterSize();
		session->Send(sendBuffer, bw.GetWriterSize());
		PlayerDBConnectionPool::GetInstance()->Push(con);
		return;
	}

	if (playerTotalCnt >= 3) 
	{
		BYTE sendBuffer[1000] = {};
		BufferWriter bw(sendBuffer);
		PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
		pktHeader->_type = PacketProtocol::S2C_CREATECHARACTER;
		code = 4;
		bw.Write(code);
		pktHeader->_pktSize = bw.GetWriterSize();
		session->Send(sendBuffer, bw.GetWriterSize());
		PlayerDBConnectionPool::GetInstance()->Push(con);
		return;
	}

	// TODO 닉네임 저장
	SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"insert into player.d_player (USER_SQ, PLAYER_NAME, PLAYER_TYPE, CREATE_DT, STRENGTH, DEXTERITY, ATTACK_RATING, DEFENSE, VITALITY, ENERGY, HP, MP,DAMAGE,SPEED) (select top 1 ?, ?, ?,GETDATE(),stat.STRENGTH,stat.DEXTERITY,stat.ATTACK_RATING,stat.DEFENSE,stat.VITALITY,stat.ENERGY,stat.HP,stat.MP, stat.DAMAGE,stat.SPEED from player.d_player_initStat stat where stat.PLAYER_TYPE = ?);", SQL_NTS);

	SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLINTEGER*)&userSQ, 0, NULL);
	SQLBindParameter(con->GetHSTMT(), 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(userName), 0, (SQLCHAR*)userName, sizeof(userName), NULL);
	SQLBindParameter(con->GetHSTMT(), 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLINTEGER*)&playerType, 0, NULL);
	SQLBindParameter(con->GetHSTMT(), 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLINTEGER*)&playerType, 0, NULL);
	retcode = SQLExecute(con->GetHSTMT());

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	{
		// 에러 핸들링
		SQLSMALLINT recNumber = 1;
		SQLWCHAR sqlstate[255];
		SQLINTEGER nativeError;
		SQLWCHAR messageText[256];
		SQLSMALLINT textLength;

		while (SQLGetDiagRec(SQL_HANDLE_STMT, con->GetHSTMT(), recNumber, sqlstate, &nativeError, messageText, sizeof(messageText), &textLength) != SQL_NO_DATA) {
			std::cerr << "SQLSTATE: " << sqlstate << std::endl;
			std::cerr << "Native Error: " << nativeError << std::endl;
			std::cerr << "Message: " << messageText << std::endl;
			recNumber++;
		}
	}

	// 캐릭터 리스트 뿌려주기 
	SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"select PLAYER_NAME, PLAYER_TYPE, LEVEL from player.d_player where USER_SQ = ? order by CREATE_DT;", SQL_NTS);
	SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLINTEGER*)&userSQ, 0, NULL);
	SQLExecute(con->GetHSTMT());
	WCHAR playerName[100] = {};
	int32 dPlayerType = 0;
	int32 level = 0;
	SQLLEN tempLen = 0;
	SQLBindCol(con->GetHSTMT(), 1, SQL_WCHAR, &playerName, sizeof(playerName), &tempLen);
	SQLBindCol(con->GetHSTMT(), 2, SQL_INTEGER, &dPlayerType, sizeof(dPlayerType), &tempLen);
	SQLBindCol(con->GetHSTMT(), 3, SQL_INTEGER, &level, sizeof(level), &tempLen);
	struct TEMP 
	{
	public:
		WCHAR playerName[50] = {0};
		int32 playerNameLen = 0;
		int32 playerType;
		int32 playerLevel;
	};

	std::vector<TEMP> v;
	while (SQLFetch(con->GetHSTMT()) != SQL_NO_DATA)
	{
		TEMP temp;
		temp.playerType = dPlayerType;
		int32 playerNameLen = wcslen(playerName) * sizeof(WCHAR);
		temp.playerNameLen = playerNameLen;
		::memcpy(temp.playerName, playerName, playerNameLen);
		temp.playerLevel = level;
		v.push_back(temp);
	}
	SQLCloseCursor(con->GetHSTMT());

	{
		BYTE sendBuffer2[1000] = {};
		BufferWriter bw2(sendBuffer2);
		PacketHeader* pktHeader = bw2.WriteReserve<PacketHeader>();
		int32 playerCnt = v.size();
		bw2.Write(playerCnt);
		for (auto& t : v) 
		{
			bw2.Write(t.playerType);
			bw2.Write(t.playerNameLen);
			bw2.WriteWString(t.playerName, t.playerNameLen);
			bw2.Write(t.playerLevel);
		}

		pktHeader->_type = PacketProtocol::S2C_CHARACTERLIST;
		pktHeader->_pktSize = bw2.GetWriterSize();

		if (playerCnt > 0)
			session->Send(sendBuffer2, bw2.GetWriterSize());
	}

	{
		BYTE sendBuffer[1000] = {};
		BufferWriter bw(sendBuffer);
		PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
		pktHeader->_type = PacketProtocol::S2C_CREATECHARACTER;
		bw.Write(code);
		pktHeader->_pktSize = bw.GetWriterSize();
		session->Send(sendBuffer, bw.GetWriterSize());
	}
	PlayerDBConnectionPool::GetInstance()->Push(con);
}

void PacketHandler::HandlePacket_C2S_CHARACTERLIST(LoginSession* session, BYTE* packet, int32 packetSize)
{
	DBConnection* con = PlayerDBConnectionPool::GetInstance()->Pop();
	int32 userSQ;
	BufferReader br(packet);
	br.Read(userSQ);

	// 캐릭터 리스트 뿌려주기 
	SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"select PLAYER_NAME, PLAYER_TYPE, LEVEL from player.d_player where USER_SQ = ? order by CREATE_DT;", SQL_NTS);
	SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLINTEGER*)&userSQ, 0, NULL);
	SQLExecute(con->GetHSTMT());
	WCHAR playerName[100] = {};
	int32 dPlayerType = 0;
	int32 level = 0;
	SQLLEN tempLen = 0;
	SQLBindCol(con->GetHSTMT(), 1, SQL_WCHAR, &playerName, sizeof(playerName), &tempLen);
	SQLBindCol(con->GetHSTMT(), 2, SQL_INTEGER, &dPlayerType, sizeof(dPlayerType), &tempLen);
	SQLBindCol(con->GetHSTMT(), 3, SQL_INTEGER, &level, sizeof(level), &tempLen);
	struct TEMP
	{
	public:
		WCHAR playerName[50] = { 0 };
		int32 playerNameLen = 0;
		int32 playerType;
		int32 playerLevel;
	};

	std::vector<TEMP> v;
	while (SQLFetch(con->GetHSTMT()) != SQL_NO_DATA)
	{
		TEMP temp;
		temp.playerType = dPlayerType;
		int32 playerNameLen = wcslen(playerName) * sizeof(WCHAR);
		temp.playerNameLen = playerNameLen;
		::memcpy(temp.playerName, playerName, playerNameLen);
		temp.playerLevel = level;
		v.push_back(temp);
	}
	SQLCloseCursor(con->GetHSTMT());

	{
		BYTE sendBuffer2[1000] = {};
		BufferWriter bw2(sendBuffer2);
		PacketHeader* pktHeader = bw2.WriteReserve<PacketHeader>();
		int32 playerCnt = v.size();
		bw2.Write(playerCnt);
		for (auto& t : v)
		{
			bw2.Write(t.playerType);
			bw2.Write(t.playerNameLen);
			bw2.WriteWString(t.playerName, t.playerNameLen);
			bw2.Write(t.playerLevel);
		}

		pktHeader->_type = PacketProtocol::S2C_CHARACTERLIST;
		pktHeader->_pktSize = bw2.GetWriterSize();

		if (playerCnt > 0)
			session->Send(sendBuffer2, bw2.GetWriterSize());
	}
	
	PlayerDBConnectionPool::GetInstance()->Push(con);
}

void PacketHandler::HandlePacket_C2S_DELETECHARACTER(LoginSession* session, BYTE* packet, int32 packetSize)
{
	DBConnection* con = PlayerDBConnectionPool::GetInstance()->Pop();
	
	int32 playerNameLen;
	WCHAR playerName[256] = {};
	int8 playerType;
	int32 userSQ;

	BufferReader br(packet);
	br.Read(playerNameLen);
	br.ReadWString(playerName,playerNameLen);
	br.Read(playerType);
	br.Read(userSQ);
	
	// 캐릭터 삭제하기
	{
		WCHAR buffer[256] = {};
		::memcpy(buffer, playerName, playerNameLen);
		SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"delete from player.d_player where PLAYER_NAME = ?;", SQL_NTS);
		SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(buffer), 0, (SQLWCHAR*)buffer, sizeof(buffer), NULL);
		SQLExecute(con->GetHSTMT());
	}

	{
		// 캐릭터 리스트 뿌려주기 
		SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"select PLAYER_NAME, PLAYER_TYPE, LEVEL from player.d_player where USER_SQ = ? order by CREATE_DT;", SQL_NTS);
		SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLINTEGER*)&userSQ, 0, NULL);
		SQLExecute(con->GetHSTMT());
		WCHAR playerName[100] = {};
		int32 dPlayerType = 0;
		int32 level = 0;
		SQLLEN tempLen = 0;
		SQLBindCol(con->GetHSTMT(), 1, SQL_WCHAR, &playerName, sizeof(playerName), &tempLen);
		SQLBindCol(con->GetHSTMT(), 2, SQL_INTEGER, &dPlayerType, sizeof(dPlayerType), &tempLen);
		SQLBindCol(con->GetHSTMT(), 3, SQL_INTEGER, &level, sizeof(level), &tempLen);
		struct TEMP
		{
		public:
			WCHAR playerName[50] = { 0 };
			int32 playerNameLen = 0;
			int32 playerType;
			int32 playerLevel;
		};

		std::vector<TEMP> v;
		while (SQLFetch(con->GetHSTMT()) != SQL_NO_DATA)
		{
			TEMP temp;
			temp.playerType = dPlayerType;
			int32 playerNameLen = wcslen(playerName) * sizeof(WCHAR);
			temp.playerNameLen = playerNameLen;
			::memcpy(temp.playerName, playerName, playerNameLen);
			temp.playerLevel = level;
			v.push_back(temp);
		}
		SQLCloseCursor(con->GetHSTMT());

		BYTE sendBuffer2[1000] = {};
		BufferWriter bw2(sendBuffer2);
		PacketHeader* pktHeader = bw2.WriteReserve<PacketHeader>();
		int32 playerCnt = v.size();
		bw2.Write(playerCnt);
		for (auto& t : v)
		{
			bw2.Write(t.playerType);
			bw2.Write(t.playerNameLen);
			bw2.WriteWString(t.playerName, t.playerNameLen);
			bw2.Write(t.playerLevel);
		}

		pktHeader->_type = PacketProtocol::S2C_CHARACTERLIST;
		pktHeader->_pktSize = bw2.GetWriterSize();
		session->Send(sendBuffer2, bw2.GetWriterSize());
	}
	PlayerDBConnectionPool::GetInstance()->Push(con);
}

void PacketHandler::HandlePacket_C2S_GAMEPLAY(LoginSession* session, BYTE* packet, int32 packetSize)
{
	DBConnection* con = PlayerDBConnectionPool::GetInstance()->Pop();

	int32 playerNameLen;
	WCHAR playerName[256] = {};
	int32 userSQ;
	BufferReader br(packet);
	br.Read(playerNameLen);
	br.ReadWString(playerName, playerNameLen);
	br.Read(userSQ);

	Vector3 pos;
	pos.y = 0;
	int32 port;
	int32 playerSQ;
	{
		SQLLEN len;
		WCHAR buffer[256] = {};
		::memcpy(buffer, playerName, playerNameLen);

		SQLPrepare(con->GetHSTMT(), (SQLWCHAR*)L"select X,Y,Z,LAST_PORT,PLAYER_SQ from player.d_player where player_name = ?;", SQL_NTS);
		SQLBindParameter(con->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(buffer), 0, (SQLWCHAR*)buffer, sizeof(buffer), NULL);
		SQLBindCol(con->GetHSTMT(), 1, SQL_C_FLOAT, &pos.x, sizeof(pos.x), &len);
		SQLBindCol(con->GetHSTMT(), 2, SQL_C_FLOAT, &pos.y, sizeof(pos.y), &len);
		SQLBindCol(con->GetHSTMT(), 3, SQL_C_FLOAT, &pos.z, sizeof(pos.z), &len);
		SQLBindCol(con->GetHSTMT(), 4, SQL_C_LONG, &port, sizeof(port), &len);
		SQLBindCol(con->GetHSTMT(), 5, SQL_C_LONG, &playerSQ, sizeof(playerSQ), &len);
		SQLExecute(con->GetHSTMT());
		SQLFetch(con->GetHSTMT());
		SQLCloseCursor(con->GetHSTMT());
	}

	if (port == 0)
	{
		// TODO 캐릭터 초기 접속
		port = 30004;
	}
	BYTE sendBuffer[1000] = {};
	BufferWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
	bw.Write((int16)port);
	bw.Write(playerSQ);
	pktHeader->_type = PacketProtocol::S2C_SERVERMOVE;
	pktHeader->_pktSize = bw.GetWriterSize();
	session->Send(sendBuffer, bw.GetWriterSize());
	PlayerDBConnectionPool::GetInstance()->Push(con);
}

void PacketHandler::HandlePacket_C2S_SERVER_MOVE(LoginSession* session, BYTE* packet, int32 packetSize)
{
	DBConnection* playerCon = PlayerDBConnectionPool::GetInstance()->Pop();
	Vector3 playerPos = { 0,0,0 };
	// 로그아웃 시간 DB에 저장
	{
		SQLLEN len;
		SQLPrepare(playerCon->GetHSTMT(), (SQLWCHAR*)L"update player.d_player set x = ?,y = ?,z = ?", SQL_NTS);
		SQLBindParameter(playerCon->GetHSTMT(), 1, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, (SQLFLOAT*)&playerPos.x, 0, NULL);
		SQLBindParameter(playerCon->GetHSTMT(), 2, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, (SQLFLOAT*)&playerPos.y, 0, NULL);
		SQLBindParameter(playerCon->GetHSTMT(), 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, (SQLFLOAT*)&playerPos.z, 0, NULL);
		SQLExecute(playerCon->GetHSTMT());
		SQLFetch(playerCon->GetHSTMT());
		SQLCloseCursor(playerCon->GetHSTMT());
	}

	// ServerMovePacket* dddddd = reinterpret_cast<ServerMovePacket*>(packet);

	int32 userSQ;
	int32 playerSQ;
	int16 serverPort;

	BufferReader br(packet);
	br.Read(userSQ);
	br.Read(playerSQ);
	br.Read(serverPort);

	BYTE sendBuffer[1000] = {};
	BufferWriter bw(sendBuffer);
	PacketHeader* pktHeader = bw.WriteReserve<PacketHeader>();
	bw.Write(serverPort);
	bw.Write(playerSQ);
	pktHeader->_type = PacketProtocol::S2C_SERVERMOVE;
	pktHeader->_pktSize = bw.GetWriterSize();
	session->Send(sendBuffer, bw.GetWriterSize());
	PlayerDBConnectionPool::GetInstance()->Push(playerCon);
}
