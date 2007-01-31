
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "dcontainer.h"
#include "dpair.h"
#include "dvector.h"

#define display_error(a,b) display_error_msg(a,b,__LINE__)

#define STMT_CATCH(a) { ret = (a); \
    if (ret==SQL_SUCCESS_WITH_INFO) display_error(SQL_HANDLE_STMT, hstmt); \
    else if (!SQL_SUCCEEDED(ret)) \
	{ \
	    fprintf(stderr, "  Error:\n"); \
	    display_error(SQL_HANDLE_STMT, hstmt); \
	    goto quit; \
	} \
    }

void display_error_msg( SQLSMALLINT type, SQLHANDLE handle, int lineno )
{
    SQLRETURN		ret;
    SQLINTEGER		i = 0, native;
    SQLCHAR		state[7], text[256];
    SQLSMALLINT		len;

    fprintf(stderr, "  odbc_crawl.c line %i:\n", lineno);

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
    int			table_cnt;
//    container		*V = vector_container( custom_container(sizeof(table_info), NULL) );
    container		*V = vector_container( pair_container( string_container(), string_container() ) );

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

    // FreeTDS krasjer på SQLDriverConnect! Derfor bruker vi SQLConnect istedet.
    ret = SQLConnect(hdbc, (SQLCHAR*) "adventure", SQL_NTS, (SQLCHAR*) "ax", SQL_NTS, (SQLCHAR*) "Ju13brz;", SQL_NTS);

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

    STMT_CATCH( SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt) );

    // Å spesifisere "TABLE" direkte fungerer ikke :\ (FreeTDS?)
    // Derfor er vi nødt til å sortere ut tabellene etterpå.
    STMT_CATCH( SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0) );

    STMT_CATCH( SQLNumResultCols(hstmt, &columns) );

    while (SQL_SUCCEEDED(ret = SQLFetch(hstmt)))
	{
	    char		buf[3][256];
	    SQLINTEGER		indicator;
	    char		*catalog, *schema, *table_name;

	    ret = SQLGetData(hstmt, 4, SQL_C_CHAR, buf[0], sizeof(buf[0]), &indicator);
	    if (SQL_SUCCEEDED(ret) && !strcmp(buf[0], "TABLE"))
		{
		    ret = SQLGetData(hstmt, 1, SQL_C_CHAR, buf[0], sizeof(buf), &indicator);
		    if (SQL_SUCCEEDED(ret) && indicator!=SQL_NULL_DATA)
			catalog = buf[0];
		    else
			catalog = NULL;

		    ret = SQLGetData(hstmt, 2, SQL_C_CHAR, buf[1], sizeof(buf), &indicator);
		    if (SQL_SUCCEEDED(ret) && indicator!=SQL_NULL_DATA)
			schema = buf[1];
		    else
			schema = NULL;

		    ret = SQLGetData(hstmt, 3, SQL_C_CHAR, buf[2], sizeof(buf), &indicator);
		    if (SQL_SUCCEEDED(ret) && indicator!=SQL_NULL_DATA)
			table_name = buf[2];
		    else
			table_name = NULL;

		    vector_pushback(V, schema, table_name);
		}
	}

    for (table_cnt=0; table_cnt<vector_size(V); table_cnt++)
	{
	    char	query[1024];
	    char	*schema = (char*)pair(vector_get(V,table_cnt)).first.ptr;
	    char	*table_name = (char*)pair(vector_get(V,table_cnt)).second.ptr;

	    printf("%s.%s:\n", schema, table_name);

	    STMT_CATCH( SQLFreeHandle(SQL_HANDLE_STMT, hstmt) );
	    STMT_CATCH( SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt) );

//	    STMT_CATCH( SQLColumns(hstmt, NULL, 0, schema, SQL_NTS, table_name, SQL_NTS, NULL, 0) );
//	    STMT_CATCH( SQLExecDirect(hstmt, "select FirstName, LastName from Person.Contact;", SQL_NTS) );

	    snprintf(query, 1023, "SELECT * FROM %s.%s;", schema, table_name);
//	    printf("%s\n", query);

	    STMT_CATCH( SQLExecDirect(hstmt, (SQLCHAR*)query, SQL_NTS) );
	    STMT_CATCH( SQLNumResultCols(hstmt, &columns) );

	    while (SQL_SUCCEEDED(ret = SQLFetch(hstmt)))
		{
		    char		buf[512];
		    SQLUSMALLINT	i;
		    SQLINTEGER		indicator;

		    for (i=1; i<=columns; i++)
			{
			    ret = SQLGetData(hstmt, i, SQL_C_CHAR, buf, sizeof(buf), &indicator);
			    if (SQL_SUCCEEDED(ret))
				{
				    if (indicator==SQL_NULL_DATA) strcpy(buf, "NULL");
				}
			    printf("| %s ", buf);
			}
		    printf("|\n");
		}
	}

    destroy(V);

quit:
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}
