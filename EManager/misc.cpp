#include "em.h"
#include "misc.h"
#include "comms.h"
#include "inet.h"


bool Misc::dbconn()
{
	if ( !mysql_init( &m_dbconn ) )
	{
		Error( "Can't initialise database connection: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	my_bool reconnect = 1;
	if ( mysql_options( &m_dbconn, MYSQL_OPT_RECONNECT, &reconnect ) )
		return false;
	string::size_type colonIndex = emConfig.DB_HOST.find( ":/" );
	if ( colonIndex != string::npos )
	{
		string dbHost = emConfig.DB_HOST.substr( 0, colonIndex );
		string dbPort = emConfig.DB_HOST.substr( colonIndex+1 );
		if ( !mysql_real_connect( &m_dbconn, dbHost.c_str(), emConfig.DB_USER.c_str(), 
			emConfig.DB_PASS.c_str(), 0, atoi(dbPort.c_str()), 0, CLIENT_FOUND_ROWS) )
		{
			Error( "Can't connect to server: %s", mysql_error( &m_dbconn ) );
			return false;
		}
	}
	else
	{
		if (!mysql_real_connect( &m_dbconn, emConfig.DB_HOST.c_str(), emConfig.DB_USER.c_str(), 
			emConfig.DB_PASS.c_str(), 0, 0, 0, CLIENT_FOUND_ROWS) )
		{
			Error( "Can't connect to server: %s", mysql_error( &m_dbconn ) );
			return false;
		}
	}

	 if (mysql_set_character_set(&m_dbconn, "utf8" ) ) 
	 { 
            Error ("set character set fail:%s" , mysql_error(&m_dbconn) ) ; 
	     return false;
        } 
	if ( mysql_select_db( &m_dbconn, emConfig.DB_NAME.c_str() ) )
	{
		Error( "Can't select database: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	return true;
}


void Misc::expire()
{
	//do something 
	Info("Misc::expire");
	Inet::closePendingSock();
	HbChk();
}

void Misc::HbChk()
{
	Debug(1,"Misc::HbChk");
	//获取心跳间隔
	unsigned int hb_interval = 60; 
	unsigned int maxtimes = 1;
	
	char sql[SQLBUFSIZE] = {0,};
	snprintf( sql, sizeof(sql), "select ifnull(jqm,''),ifnull(IPAddress,'') from zj_table where  TIMESTAMPDIFF(SECOND,updateDate,now()) > %u and  TIMESTAMPDIFF(SECOND,updateDate,now()) < %u and status = 1",1*hb_interval,3*hb_interval);
	Info("%s",sql);
	if ( mysql_query( &m_dbconn, sql ) )
	{
	     Error( "Can't run query: %s", mysql_error( &m_dbconn));
	     return ;
	}
	//get result
	MYSQL_RES *result = mysql_store_result( &m_dbconn );
	if ( !result )
	{
		Error( "Can't use query result: %s", mysql_error( &m_dbconn ) );
		return;
	}
	int n_monitors = mysql_num_rows( result );
	Debug( 1, "Got %d managers", n_monitors );

	for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	{
		string jqm = dbrow[0];
		string ip = dbrow[1];
		if(!jqm.empty() || !ip.empty())
		{
			//令报心跳
			string cmd = (const char *)config.Item(m_dbconn,E_CFG_D_HB);
			TcpInetSocket * pSocket =  Inet::findConnSockAll(jqm,ip);
			//send cmd
			Info("No hb come,force it %s,%s",jqm.empty()?"":jqm.c_str(),ip.empty()?"":ip.c_str());
			if(pSocket && (!cmd.empty() && !Inet::sendCmd(pSocket,cmd)))
			{
				Error("send cmd error");
			}

		}
		
	}
	mysql_free_result( result );
	//仍未上报
	snprintf( sql, sizeof(sql), "select ifnull(jqm,''),ifnull(IPAddress,'') from zj_table where  TIMESTAMPDIFF(SECOND,updateDate,now()) > %u and status = 1 and wlzt <> 2",3*hb_interval);
	Info("%s",sql);
	if ( mysql_query( &m_dbconn, sql ) )
	{
	     Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
	     return ;
	}
	//get result
	result = mysql_store_result( &m_dbconn );
	if ( !result )
	{
		Error( "Can't use query result: %s", mysql_error( &m_dbconn ) );
		return;
	}
	n_monitors = mysql_num_rows( result );
	Debug( 1, "Got %d managers", n_monitors );

	for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++)
	{
		string jqm = dbrow[0];
		string ip = dbrow[1];
		if(!jqm.empty() ||!ip.empty())
		{
			//关闭socket
			Inet::delUnuseSockAll(jqm,ip);			
			//置设备离线
			memset(sql,0,SQLBUFSIZE);
			snprintf( sql, sizeof(sql), "update  zj_table set  wlzt = 2 where jqm = '%s' and status = 1 and wlzt <> 2", jqm.empty()?"":jqm.c_str());
			Info("%s",sql);
			if ( mysql_query( &m_dbconn, sql))
			{
				mysql_free_result( result );
				Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
				return ;
			}	
			//通知前端	
		}
	}
	mysql_free_result( result );
	
}



void CheckMethod::expire()
{
	//do something 
	//$0aSbZt0xxxx
	//令报设备状态
	Info("CheckMethod::expire");
	// 1. 读取主机检测方式

	// 2. 读取设备检测方式
	// 3. 读取检测方式类型
	// 3.1 时段性检测当前是否在计划时段内?若是
	// 检测运行状态待机、运行? 若在待机则执行开始命令。若不在计划时间内，运行
	// 状态在运行状态则执行停止命令。

	// 3.2 立即性检测运行状态为待机，则执行开始命令

	
	

}


int CheckMethod::getZjChkMethod()
{
	string startCmd;
	string stopCmd;
	string pauseCmd;
	string upErrCmd;
	string clearErrCmd;
	
	//zj_table

	//jcfs_table

	return 0;
	
}


int CheckMethod::getSbChkMethod()
{
	return 0;
}
