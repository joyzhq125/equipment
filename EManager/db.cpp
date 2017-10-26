

#include <stdlib.h>
#include <string>

#include "em.h"
#include "logger.h"
#include "cfg.h"
#include "db.h"

using namespace std;

MYSQL g_dbconn;
bool g_dbConnected = false;

void dbConnect()
{
	if ( !mysql_init( &g_dbconn ) )
	{
		Error( "Can't initialise database connection: %s", mysql_error( &g_dbconn ) );
		exit( mysql_errno( &g_dbconn ) );
	}
	my_bool reconnect = 1;
	if ( mysql_options( &g_dbconn, MYSQL_OPT_RECONNECT, &reconnect ) )
		Fatal( "Can't set database auto reconnect option: %s", mysql_error( &g_dbconn ) );
	string::size_type colonIndex = emConfig.DB_HOST.find( ":/" );
	if ( colonIndex != string::npos )
	{
		string dbHost = emConfig.DB_HOST.substr( 0, colonIndex );
		string dbPort = emConfig.DB_HOST.substr( colonIndex+1 );
		if ( !mysql_real_connect( &g_dbconn, dbHost.c_str(), emConfig.DB_USER.c_str(), 
			emConfig.DB_PASS.c_str(), 0, atoi(dbPort.c_str()), 0, CLIENT_FOUND_ROWS) )
		{
			Error( "Can't connect to server: %s", mysql_error( &g_dbconn ) );
			exit( mysql_errno( &g_dbconn ) );
		}
	}
	else
	{
		if (!mysql_real_connect( &g_dbconn, emConfig.DB_HOST.c_str(), emConfig.DB_USER.c_str(), 
			emConfig.DB_PASS.c_str(), 0, 0, 0, CLIENT_FOUND_ROWS) )
		{
			Error( "Can't connect to server: %s", mysql_error( &g_dbconn ) );
			exit( mysql_errno( &g_dbconn ) );
		}
	}

	 if (mysql_set_character_set(&g_dbconn, "utf8" ) ) 
	 { 
            Error ("set character set fail:%s" , mysql_error(&g_dbconn) ) ; 
	     exit( mysql_errno( &g_dbconn ) );
        } 
	unsigned long mysqlVersion = mysql_get_server_version( &g_dbconn );
	Info("mysqlVersion is %ld",mysqlVersion);
	if ( mysqlVersion < 50019 )
	{
		if ( mysql_options( &g_dbconn, MYSQL_OPT_RECONNECT, &reconnect ) )
			Fatal( "Can't set database auto reconnect option: %s", mysql_error( &g_dbconn ) );
	}
	if ( mysql_select_db( &g_dbconn, emConfig.DB_NAME.c_str() ) )
	{
		Error( "Can't select database: %s", mysql_error( &g_dbconn ) );
		exit( mysql_errno( &g_dbconn ) );
	}
	g_dbConnected = true;
}

