
#include "em.h"
#include "ecdr.h"
#include "comms.h"
#include "time.h"
#include "msghdl.h"
#include "msgqueue.h"

#include "rspdata.h"
#include "reqdata.h"
#include "commrec.h"
#include "inet.h"


 ReqData*  ReqData::preProc(St_MsgReq &stMsgReq,MYSQL &dbconn)
{
	Info("ReqData::preProc");
	char* pdata = stMsgReq.recvData;
	pdata += REQ_TYPEID_ADDR;
	if(NULL != pdata && *pdata == CMD_ORG)
	{
		return new ReqCmdOrg(stMsgReq,dbconn);
	}
	//else if(NULL != pdata && (*pdata == CMD_DIGIT_ZS || *pdata == CMD_DIGIT_XS))
	//{
	//	return new ReqCmdDigit(stMsgReq,dbconn,(E_CMD_TYPE)*pdata);
	//}
	else if(NULL != pdata && *pdata == CMD_NORMAL)
	{
		return new ReqCmd(stMsgReq,dbconn);
	}
	//else if(NULL != pdata && *pdata == CMD_ID)
	//{
	//	return new ReqCmdId(stMsgReq,dbconn);
	//}
	else if(NULL != pdata)
	{
		Error("unknow cmdtype %d",*pdata);
		return NULL;
	}
	else
	{
		Error("unknow cmdtype the pointer is null");
		return NULL;
	}
}


bool ReqData::getJqm(unsigned devid,string& jqm,string& ip)
{
	Info("ReqData::getJqm");
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	snprintf( sql, sizeof(sql), "select ifnull(jqm,''),ifnull(IPAddress,'') from zj_table where id = %u and status = 1",devid);
	Info("%s",sql);	
	//string jqm;
	if ( mysql_query( &m_dbconn, sql ) )
	{
	     Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
	     return false;
	}
	//get result
	MYSQL_RES *result = mysql_store_result( &m_dbconn );
	if ( !result )
	{
		Error( "Can't use query result: %s", mysql_error( &m_dbconn ) );
		return false;
	}

	int n_items = mysql_num_rows( result );
	if ( n_items != 1 )
	{
		mysql_free_result( result );
		Error( "get rows num is %d", n_items );
		return false;
	}
	//find devid (db)
	MYSQL_ROW dbrow = mysql_fetch_row( result );
	//for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	if (mysql_errno( &m_dbconn ))
	{
		mysql_free_result( result );
		Error( "Can't fetch row: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	if (dbrow)
	{
		jqm = dbrow[0];
		ip = dbrow[1];
		Info("ReqCmdOrg::doProc jqm %s,ip is %s",jqm.empty()?"":jqm.c_str(),ip.empty()?"":ip.c_str());
	}
	else
	{
		mysql_free_result( result );
		Error( "dbrow is null");
		return false;		
	}
	mysql_free_result( result );
	return true;

}


bool ReqCmdOrg::doProc()
{
	OperationRec* pOperRec = NULL;
	//device info
	unsigned int    devid;
	//char 		*pCmd = NULL;
	string 		jqm;
	string 		ip;
	unsigned short port;
	char cmd[EM_CMD_LEN+1] = {0};

	char* pdata = m_stMsgReq.recvData;

	St_ecdrDecoder *pstDecoder = NULL;

	Info("ReqCmdOrg::doProc");

	//get dev id
	pstDecoder = ecdr_CreateDecoder((unsigned char *)pdata,EM_REQ_LEN);
	ecdr_DecodeUint32(pstDecoder, (unsigned int*)&devid);
	ecdr_DestroyDecoder(pstDecoder);
	
	Info("devid %u",devid);

	//get cmd
	if(NULL == (pdata + REQ_CMD_ADDR))
	{
		Error("null pointer");
		return false;
		
	}
	//// 12bits	
	memcpy(cmd,pdata + REQ_CMD_ADDR,EM_CMD_LEN);
	
	//add req to list
#ifdef REQ_REPLY	
	St_ReqReply *pstReqReply = new St_ReqReply;//= {0,};
	memset(pstReqReply,0,sizeof(St_ReqReply));
	pstReqReply->devid = devid;
	memcpy(pstReqReply->stReq.recvData,cmd,EM_CMD_LEN);
	pstReqReply->sndTime = tvNow();
	MsgReqThread::m_reqCmdList.push_back(pstReqReply);
	
	 char ReqTemp[EM_CMD_LEN+1] = {0,};
	 memcpy(ReqTemp,pstReqReply->stReq.recvData,EM_CMD_LEN);
	 Info("ReqCmdOrg::doProc  reqtemp is %s",ReqTemp);
#endif			 
	Info("ReqCmdOrg::doProc cmd is %s",cmd);
	
	//store operation
	string orgcmd = cmd;
	pOperRec = new OperationRec(m_dbconn,devid,CMD_ORG,orgcmd);
	if(pOperRec)
	{
		int rowid = 0;
		pOperRec->saveTodb(rowid);
		delete pOperRec;
	}
	if(!getJqm(devid,jqm,ip) ||(jqm.empty() && ip.empty()))
	{
		Error("getJqm fail");
		return false;
	}

	TcpInetSocket * pSocket =  Inet::findConnSockAll(jqm,ip);
	//send cmd
	if(!pSocket || !Inet::sendCmd(pSocket,string(cmd)))
	{
		Error("send cmd %s error",cmd);
	}
	//命令主机上报设备全码	
	string cmd_devcode = (const char *)config.Item(m_dbconn,E_CFG_D_DEVCODE );	
	string devhdr = "$&";
	if(startsWith(cmd,devhdr) /* && !Inet::sendCmd(pSocket,cmd_devcode)*/)
	{
		pSocket->m_sbjqm= "";
		//force up devcode
		//Error("send cmd_devcode %s error",cmd_devcode.c_str());
	}

	return true;
	
}


bool ReqCmdId::doProc()
{
	Info("ReqCmdId::doProc");
	return true;
}

