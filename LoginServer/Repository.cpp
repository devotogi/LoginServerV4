#include "pch.h"
#include "Repository.h"

void Repository::FindUserByUserId(WCHAR* userId, const UserInfo& userInfo)
{
	Lock lock(&_cs);
	WCHAR buffer[256] = {};
	int32 userIdLen = wcslen(userId);
	::memcpy(buffer, userId, sizeof(WCHAR) * userIdLen);

	SQLPrepare(_hStmt, (SQLWCHAR*)L"select top 1 user_id , user_pw from requies.d_user where user_id = ? ;", SQL_NTS);
	SQLBindParameter(_hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, sizeof(buffer), 0, (SQLWCHAR*)buffer, sizeof(buffer), NULL);

	WCHAR userId2;
	SQLLEN len = 0;
	SQLBindCol(_hStmt, 1, SQL_WCHAR, (SQLWCHAR*)&userInfo.userId, sizeof(userInfo.userId), &len);
	SQLBindCol(_hStmt, 2, SQL_WCHAR, (SQLWCHAR*)&userInfo.userPw, sizeof(userInfo.userPw), &len);
	
	SQLExecute(_hStmt);
	SQLFetch(_hStmt);
	SQLCloseCursor(_hStmt);
}

Repository::Repository()
{
	InitializeCriticalSection(&_cs);

	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv) != SQL_SUCCESS)
		printf("ERROR\n");

	if (SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS)
		printf("ERROR\n");

	if (SQLAllocHandle(SQL_HANDLE_DBC, _hEnv, &_hDbc) != SQL_SUCCESS)
		printf("ERROR\n");

	SQLConnect(_hDbc, _ODBC_Name, SQL_NTS, _ODBC_ID, SQL_NTS, _ODBC_PW, SQL_NTS);

	if (SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt) != SQL_SUCCESS)
		printf("ERROR\n");

}

Repository::~Repository()
{
	DeleteCriticalSection(&_cs);
	SQLFreeHandle(SQL_HANDLE_STMT, _hStmt);
	SQLDisconnect(_hDbc);
	SQLFreeHandle(SQL_HANDLE_DBC, _hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);
}
