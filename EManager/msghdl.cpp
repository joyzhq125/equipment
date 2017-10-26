
#include "em.h"
#include "time.h"
#include "db.h"
#include "msghdl.h"
#include "msgqueue.h"
#include "reqdata.h"
#include "rspdata.h"
#include "comms.h"


ReqList MsgReqThread::m_reqCmdList;

bool MsgThread::dbconn()
{
	if ( !mysql_init( &m_DbConnection ) )
	{
		Error( "Can't initialise database connection: %s", mysql_error( &m_DbConnection ) );
		return false;
	}
	my_bool reconnect = 1;
	if ( mysql_options( &m_DbConnection, MYSQL_OPT_RECONNECT, &reconnect ) )
		return false;
	string::size_type colonIndex = emConfig.DB_HOST.find( ":/" );
	if ( colonIndex != string::npos )
	{
		string dbHost = emConfig.DB_HOST.substr( 0, colonIndex );
		string dbPort = emConfig.DB_HOST.substr( colonIndex+1 );
		if ( !mysql_real_connect( &m_DbConnection, dbHost.c_str(), emConfig.DB_USER.c_str(), 
			emConfig.DB_PASS.c_str(), 0, atoi(dbPort.c_str()), 0, CLIENT_FOUND_ROWS) )
		{
			Error( "Can't connect to server: %s", mysql_error( &m_DbConnection ) );
			return false;
		}
	}
	else
	{
		if (!mysql_real_connect( &m_DbConnection, emConfig.DB_HOST.c_str(), emConfig.DB_USER.c_str(), 
			emConfig.DB_PASS.c_str(), 0, 0, 0, CLIENT_FOUND_ROWS) )
		{
			Error( "Can't connect to server: %s", mysql_error( &m_DbConnection ) );
			return false;
		}
	}

	 if (mysql_set_character_set(&m_DbConnection, "utf8" ) ) 
	 { 
            Error ("set character set fail:%s" , mysql_error(&m_DbConnection) ) ; 
	     return false;
        } 
	if ( mysql_select_db( &m_DbConnection, emConfig.DB_NAME.c_str() ) )
	{
		Error( "Can't select database: %s", mysql_error( &m_DbConnection ) );
		return false;
	}
	return true;
}

int MsgReqThread::run()
{
	St_MsgReq stMsgReq = {0,};
	Info("MsgReqThread::run");
	while(!g_terminate)
	{
		memset(&stMsgReq,0,sizeof(St_MsgReq));
		if (1 == GetMsgReq(&stMsgReq))
		{			 
			 ReqData *reqData = ReqData::preProc(stMsgReq,m_DbConnection);
			 if(NULL != reqData)
			 {
			 	if(!reqData->doProc())
		 		{
		 			Error("MsgReqThread::run doProc fail");
		 		}
				delete reqData;
			 }
		}
		else
		{
			Error("GetMsgReq ERR");
		}

	}
	Info(" MsgReqThread::term");
	return 0;
}

int MsgRspThread::run()
{
	St_MsgRsp stMsgRsp = {0,};
	Info("MsgRspThread::run");	
	while(!g_terminate)
	{
		memset(&stMsgRsp,0,sizeof(St_MsgRsp));
		if(1 == GetMsgRsp(&stMsgRsp))
		{
			//if(stMsgRsp.tcpSrv->isClosed())
			//{
				//Info("MsgRspThread::run get  close sd discard");
				//continue;
			//}
			char tempbuffer[EM_CMD_LEN+1] = {0,};
			memcpy(tempbuffer, stMsgRsp.recvData, stMsgRsp.recvLen);
			stMsgRsp.tcpSrv->pipe_write(tempbuffer,stMsgRsp.recvLen);
			/*while*/if (stMsgRsp.tcpSrv->check_read_data((unsigned char*)stMsgRsp.recvData))
			{
				RspData   * rspData = RspData::preProc(stMsgRsp,m_DbConnection);
				if(NULL != rspData)
				{
					if(!rspData->doProc())
					{
						Error("MsgRspThread::run doProc fail");
					}
					delete rspData;
				}
			}
	
		}
		else
		{
			Error("GetMsgRsp ERR");
		}
		
	}
	Info(" MsgRspThread::term");
	return 0;
}





