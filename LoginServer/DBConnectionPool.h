#pragma once
class DBConnection
{
private:
	SQLHDBC _hDbc;
	SQLHSTMT _hStmt;

public:
	DBConnection(SQLHENV hEnv, WCHAR* odbcName, WCHAR* odbcId, WCHAR* odbcPw);
	~DBConnection();

	SQLHDBC& GetHDBC();
	SQLHSTMT& GetHSTMT();
};

class DBConnectionPool 
{
protected:
	SQLHENV						_hEnv;
	SQLHDBC						_hDbc;
	SQLHSTMT					_hStmt;
	SQLWCHAR					_ODBC_Name[256] = {};
	SQLWCHAR					_ODBC_ID[256] = {};
	SQLWCHAR					_ODBC_PW[256] = {};
	std::queue<DBConnection*>	_dbConnections;
	CriticalSectionObject		_cs;

public:
	DBConnectionPool();
	virtual ~DBConnectionPool();

	virtual void Init(const WCHAR* odbcName, const WCHAR* id, const WCHAR* password, int32 totalDbConnection);
	virtual void Push(DBConnection* con);
	virtual DBConnection* Pop();
};