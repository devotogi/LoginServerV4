#pragma once
class Repository
{
protected:
	SQLHENV					_hEnv;
	SQLHDBC					_hDbc;
	SQLHSTMT				_hStmt;
	SQLWCHAR*				_ODBC_Name = (SQLWCHAR*)L"MSSQL";
	SQLWCHAR*				_ODBC_ID = (SQLWCHAR*)L"sa";
	SQLWCHAR*				_ODBC_PW = (SQLWCHAR*)L"root";
	CRITICAL_SECTION		_cs;

public:
	static Repository* GetInstance()
	{
		static Repository repository;
		return &repository;
	}

	void FindUserByUserId(WCHAR* userId, const UserInfo& userInfo);

	Repository();
	~Repository();
};