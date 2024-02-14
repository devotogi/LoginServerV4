#include "pch.h"
#include "DBConnectionPool.h"

DBConnection::DBConnection(SQLHENV hEnv, WCHAR* odbcName, WCHAR* odbcId, WCHAR* odbcPw)
{
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS)
		printf("ERROR\n");

	if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS)
		printf("ERROR\n");

	if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &_hDbc) != SQL_SUCCESS)
		printf("ERROR\n");

	SQLConnect(_hDbc, odbcName, SQL_NTS, odbcId, SQL_NTS, odbcPw, SQL_NTS);

	if (SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt) != SQL_SUCCESS)
		printf("ERROR\n");
}

DBConnection::~DBConnection()
{
	SQLFreeHandle(SQL_HANDLE_STMT, _hStmt);
	SQLDisconnect(_hDbc);
	SQLFreeHandle(SQL_HANDLE_DBC, _hDbc);
}

SQLHDBC& DBConnection::GetHDBC()
{
	return _hDbc;
}

SQLHSTMT& DBConnection::GetHSTMT()
{
	return _hStmt;
}

DBConnectionPool::DBConnectionPool()
{
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv) != SQL_SUCCESS)
		printf("ERROR\n");

	if (SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS)
		printf("ERROR\n");
}

DBConnectionPool::~DBConnectionPool()
{
	SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);

	while (_dbConnections.empty() == false)
	{
		DBConnection* con = _dbConnections.front();
		_dbConnections.pop();
		if (con)
			delete con;
	}
}

void DBConnectionPool::Init(const WCHAR* odbcName, const WCHAR* id, const WCHAR* password, int32 totalDbConnection)
{
	wcscpy_s(_ODBC_Name, odbcName);
	wcscpy_s(_ODBC_ID, id);
	wcscpy_s(_ODBC_PW, password);

	for (int i = 0; i < totalDbConnection; i++)
	{
		DBConnection* con = new DBConnection(_hEnv, _ODBC_Name, _ODBC_ID, _ODBC_PW);
		_dbConnections.push(con);
	}
}

void DBConnectionPool::Push(DBConnection* con)
{
	LockGuard lock (&_cs);
	_dbConnections.push(con);
}

DBConnection* DBConnectionPool::Pop()
{
	LockGuard lock(&_cs);

	DBConnection* ret = nullptr;
	if (_dbConnections.empty())
	{
		ret = new DBConnection(_hEnv, _ODBC_Name, _ODBC_ID, _ODBC_PW);
	}
	else
	{
		ret = _dbConnections.front();
		_dbConnections.pop();
	}

	return ret;
}
