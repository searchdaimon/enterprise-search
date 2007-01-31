
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

#define display_error(a,b) display_error_msg(a,b,__LINE__)

void display_error_msg( SQLSMALLINT type, SQLHANDLE handle, int lineno )
{
    SQLRETURN		ret;
    SQLINTEGER		i = 0, native;
    SQLCHAR		state[7], text[256];
    SQLSMALLINT		len;

    fprintf(stderr, "  test_connect.c line %i:\n", lineno);

    do
	{
	    ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);

	    if (SQL_SUCCEEDED(ret))
		{
		    fprintf(stderr, "%s:%ld:%ld:%s\n", state, i, native, text);
		}
	}
	while (ret == SQL_SUCCESS);
}


int main(int argc, char *argv[])
{
    SQLHENV		henv;
    SQLHDBC		hdbc;
    SQLHSTMT		hstmt;
    SQLRETURN		ret;
    SQLCHAR		dbms_name[256], dbms_ver[256];
    SQLSMALLINT		columns;
    int			row = 0;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (!SQL_SUCCEEDED(ret))
	{
	    display_error(SQL_HANDLE_ENV, henv);
	    exit(-1);
	}

    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(ret))
	{
	    display_error(SQL_HANDLE_ENV, henv);
	    SQLFreeHandle(SQL_HANDLE_ENV, henv);
	    exit(-1);
	}

    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (!SQL_SUCCEEDED(ret))
	{
	    display_error(SQL_HANDLE_ENV, henv);
	    SQLFreeHandle(SQL_HANDLE_ENV, henv);
	    exit(-1);
	}

//    SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER*)5, 0);

    // FreeTDS krasjer på SQLDriverConnect! Derfor bruker vi SQLConnect istedet.
    ret = SQLConnect(hdbc, (SQLCHAR*) "adventure", SQL_NTS, (SQLCHAR*) "ax", SQL_NTS, (SQLCHAR*) "Ju13brz;", SQL_NTS);
/*
    ret = SQLDriverConnect(hdbc, NULL, "DSN=adventure;UID=ax;PWD={Ju13brz;}", SQL_NTS,
	output, sizeof(output), &outputlen,
	SQL_DRIVER_COMPLETE);
*/

    if (ret==SQL_SUCCESS_WITH_INFO)
	display_error(SQL_HANDLE_DBC, hdbc);

    if (!SQL_SUCCEEDED(ret))
	{
	    display_error(SQL_HANDLE_DBC, hdbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	    SQLFreeHandle(SQL_HANDLE_ENV, henv);
	    exit(-1);
	}

    fprintf(stderr, "ODBC Crawler: Connected to '%s'!\n", "adventure");

    SQLGetInfo(hdbc, SQL_DBMS_NAME, (SQLPOINTER)dbms_name, sizeof(dbms_name), NULL);
    SQLGetInfo(hdbc, SQL_DBMS_VER, (SQLPOINTER)dbms_ver, sizeof(dbms_ver), NULL);

    fprintf(stderr, "DBMS Name: %s\n", dbms_name);
    fprintf(stderr, "DBMS Version: %s\n", dbms_ver);

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret!=SQL_SUCCESS) display_error(SQL_HANDLE_STMT, hstmt);

//    ret = SQLTables(hstmt, SQL_ALL_CATALOGS, SQL_NTS, "", 0, "", 0, NULL, 0);
//    ret = SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, "TABLE", SQL_NTS);
//    ret = SQLTables(hstmt, NULL, 0, NULL, 0, "tablename", SQL_NTS, NULL, 0);
//    ret = SQLExecDirect(hstmt, "select FirstName, LastName from Person.Contact;", SQL_NTS);
//    ret = SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
//    ret = SQLExecDirect(hstmt, "select name, type from AdventureWorks.sys.tables;", SQL_NTS);
    ret = SQLColumns(hstmt, NULL, 0, NULL, 0, "Individual", SQL_NTS, NULL, 0);
    if (ret!=SQL_SUCCESS) display_error(SQL_HANDLE_STMT, hstmt);

    ret = SQLNumResultCols(hstmt, &columns);
    if (ret!=SQL_SUCCESS) display_error(SQL_HANDLE_STMT, hstmt);

    while (SQL_SUCCEEDED(ret = SQLFetch(hstmt)))
	{
	    int			colw[5] = {14,15,45,5,4};
//	    int			colw[5] = {45,2,3,3,3};
	    SQLUSMALLINT	i;

//	    printf("Row: %i\n", row++);
	    row++;

	    for (i=1; i<=columns; i++)
		{
		    SQLINTEGER	indicator;
		    char	buf[512];
		    ret = SQLGetData(hstmt, i, SQL_C_CHAR, buf, sizeof(buf), &indicator);
		    if (SQL_SUCCEEDED(ret))
			{
			    if (indicator==SQL_NULL_DATA) strcpy(buf, "NULL");
//			    printf("  Column %u: %s\n", i, buf);
//			    printf("| %-*.*s ", colw[i-1], colw[i-1], buf);
			    printf("| %-10.10s ", buf);
/*
			    if (i<=3)
				printf("| %-16.16s ", buf);
			    else
				printf("| %-5.5s ", buf);
*/
			}
		}

	    printf("|\n");
	}

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}
