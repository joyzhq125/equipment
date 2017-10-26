
#include "em.h"
#include "ecdr.h"
#include "emanager.h"
#include "msghdl.h"
#include "msgqueue.h"
#include "reqdata.h"
#include "rspdata.h"

#include "comms.h"
#include "commrec.h"
#include "time.h"
#include "inet.h"

#define HDR_SIZE        10
#define ADDR_SIZE      64
#define PROPER_SIZE   64


//
struct st_proto_hdr_map
{
	E_PROTO_HDR hdr;
	char value;

}proto_hdr_map[HDR_SIZE]=
{
	{E_HDR_NORMAL,'$'},
	{E_HDR_REPLY,'@'}
};

//
struct st_proto_addr_map
{
	E_PROTO_ADDR addr;
	char value;

}proto_addr_map[ADDR_SIZE]=
{
	{E_ADDR_NONEED,'0'},
	{E_ADDR_DEVCODE,'#'}
};
//
struct st_proto_proper_map
{
	E_PROTO_PROPER proper;
	char value ;
}proto_proper_map[PROPER_SIZE] = 
{
	{E_PROTO_DIGIT_ZS,'a'},
	{E_PROTO_DIGIT_XS,'b'},
	{E_PROTO_CMD,'c'},
	{E_PROTO_HB,'d'},
	{E_PROTO_ALM,'e'},
	{E_PROTO_CFG_ZS,'f'},
	{E_PROTO_CFG_XS,'h'},	
	{E_PROTO_NEG_CFG_ZS,'i'},
	{E_PROTO_NEG_CFG_XS,'j'},
	{E_PROTO_NEG_DIGIT_ZS,'m'},
	{E_PROTO_NEG_DIGIT_XS,'n'}
};


E_PROTO_HDR find_hdr(char cValue)
{

	for(int idx = 0;idx < sizeof(proto_hdr_map)/sizeof(struct st_proto_hdr_map);idx ++)
	{
		if (cValue == proto_hdr_map[idx].value)
		{
			return proto_hdr_map[idx].hdr;
		}
	}
	return E_HDR_NONE;
}

E_PROTO_ADDR find_addr(char cValue)
{

	for(int idx = 0;idx < sizeof(proto_addr_map)/sizeof(struct st_proto_addr_map);idx ++)
	{
		if (cValue == proto_addr_map[idx].value)
		{
			return proto_addr_map[idx].addr;
		}
	}
	return E_ADDR_NONE;
}

E_PROTO_PROPER find_proper(char cValue)
{

	for(int idx = 0;idx < sizeof(proto_proper_map)/sizeof(struct st_proto_proper_map);idx ++)
	{
		if (cValue == proto_proper_map[idx].value)
		{
			return proto_proper_map[idx].proper;
		}
	}
	return E_PROTO_NONE;
}

 E_PROTO_HDR RspData::m_hdr = E_HDR_NONE;
 E_PROTO_ADDR RspData::m_addr = E_ADDR_NONE;;
 E_PROTO_PROPER RspData::m_proper = E_PROTO_NONE;
 string RspData::m_def = "";
 unsigned short RspData::m_td = 0;
 string RspData::m_sdata = "";
 float   RspData::m_data = 0;
 string RspData::m_orgcmd = "";
 RspData* RspData::preProc(St_MsgRsp &stMsgRsp,MYSQL &dbconn)
{
#if 1
	Info("RspData::preProc");
	if (getProtoDtl(stMsgRsp,m_hdr,m_addr,m_proper,m_def,m_td,m_sdata,m_data))
	{
		Info("hdr = %d,addr=%d,proper=%d,def=%s,td=%d,sdata=%s,data=%f",m_hdr,m_addr,m_proper,m_def.empty()?"":m_def.c_str(),m_td,m_sdata.empty()?"":m_sdata.c_str(),m_data);
		if(E_HDR_REPLY == m_hdr)
		{
			return new RspReply(stMsgRsp,dbconn);
		}
		else if(E_HDR_NORMAL == m_hdr)
		{
			if(E_ADDR_DEVCODE == m_addr)
			{
				return new RspDevCode(stMsgRsp,dbconn);
			}
			else if(E_ADDR_NONEED == m_addr)
			{
				if(E_PROTO_HB == m_proper)
				{
					//状态也属于此类
					return new RspHB(stMsgRsp,dbconn);
				}
				else if(E_PROTO_ALM == m_proper)
				{
					return new RspAlm(stMsgRsp,dbconn);
				}
				else if(E_PROTO_CFG_ZS == m_proper ||
					E_PROTO_CFG_XS == m_proper ||
					E_PROTO_NEG_CFG_ZS == m_proper ||
					E_PROTO_NEG_CFG_XS == m_proper)
				{
					return new RspCfg(stMsgRsp,dbconn);
				}
				else if(E_PROTO_CMD == m_proper)
				{
					return new RspCmd(stMsgRsp,dbconn);
				}
				else if(E_PROTO_DIGIT_ZS == m_proper || 
					E_PROTO_DIGIT_XS  == m_proper || 
					E_PROTO_NEG_DIGIT_ZS  == m_proper || 
					E_PROTO_NEG_DIGIT_XS  == m_proper)
				{
					return new RspDigit(stMsgRsp,dbconn,m_proper);
				}
				else
				{
					Error("do nothing");
					return NULL;					
				}

				
			}
			else 
			{
				Error("do nothing");
				return NULL;
			}
		}
		else
		{
			Error("do nothing");
			return NULL;
		}
	}
	else
	{
		Error("do nothing");
		return NULL;
	}
#else
	char tempbuf[EM_CMD_LEN+1] = {0};
	char* pdata = stMsgRsp.recvData;
	strncpy(tempbuf,pdata,EM_CMD_LEN);
	Info("RspData::preProc buf is %s",tempbuf);
	if(NULL != pdata &&  find_hdr(*pdata) == E_HDR_NORMAL)
	{
		if(NULL != pdata + PRO_ADDR_ADD && find_addr(*(pdata + PRO_ADDR_ADD)) == E_ADDR_DEVCODE) //设备码
		{
			return new RspDevCode(stMsgRsp,dbconn);
		}
		else if(NULL != pdata + PRO_ADDR_ADD  && find_addr(*(pdata + PRO_ADDR_ADD)) == E_ADDR_NONEED)
		{
			if(NULL != pdata + PRO_PROPER_ADD && find_proper(*(pdata + PRO_PROPER_ADD)) == E_PROTO_HB) //心跳
			{
				//状态也属于此类
				return new RspHB(stMsgRsp,dbconn);
				
			}
			else if(NULL !=  pdata + PRO_PROPER_ADD && find_proper(*(pdata + PRO_PROPER_ADD)) == E_PROTO_ALM) //警情
			{			
				return new RspAlm(stMsgRsp,dbconn);
					
			}
			else if(NULL != pdata  + PRO_PROPER_ADD && find_proper(*(pdata + PRO_PROPER_ADD)) == E_PROTO_CFG) //配置
			{
				return new RspCfg(stMsgRsp,dbconn);
			}
			else if(NULL != pdata + PRO_PROPER_ADD  && (find_proper(*(pdata + PRO_PROPER_ADD)) == E_PROTO_DIGIT_ZS 
				|| find_proper(*(pdata + PRO_PROPER_ADD)) == E_PROTO_DIGIT_XS))
			{
				/*功能定义*/
				//查找数据库定义,可增加修改并提醒
				return new RspDigit(stMsgRsp,dbconn,find_proper(*(pdata + PRO_PROPER_ADD)));
			}
			else if(NULL != pdata)
			{
				Error("not support protocol the proporities is %c",*pdata);
			}
			else
			{
				Error("not support protocol the pointer is null");
			}
		}
		else if(NULL != pdata + PRO_ADDR_ADD)
		{
			Error("not support protocol the addr is %c",*pdata);
			return NULL;
		}
		else
		{
			Error("not support protocol the pointer is null");
		}
		
	}
	else if(NULL != pdata && find_hdr(*pdata) == E_HDR_REPLY) //reply
	{
		return new RspReply(stMsgRsp,dbconn);
	}
	else if(NULL != pdata)
	{
		Error("unknown protocol header %c",*pdata);
		return NULL;
	}
	else
	{
		Error("not support protocol the pointer is null");
	}
#endif
}

bool RspData::getProtoDtl(St_MsgRsp &stMsgRsp,E_PROTO_HDR &hdr,
							E_PROTO_ADDR &addr,E_PROTO_PROPER &proper,string& def,
							unsigned short& td,string& sdata,float &data)
{
	char* pdata = stMsgRsp.recvData;
	char tempbuf[EM_CMD_LEN+1] = {0};
	bool res = true;
	sdata = "";
	data = 0;
	memcpy(tempbuf,pdata,EM_CMD_LEN);
	hdr = E_HDR_NONE;
	addr = E_ADDR_NONE;
	proper = E_PROTO_NONE;
	def = "";
	td = 0;
	m_orgcmd = tempbuf;
	Info("RspData::getProtoDtl buf is %s",tempbuf);
	do
	{
		if(NULL != pdata + PRO_HDR_ADD)
		{
		 	hdr = find_hdr(*pdata);
			
			if(E_HDR_NONE == hdr)
			{
				Error("unknown protocol header %c",*(pdata + PRO_HDR_ADD));
				res =  false;
				break;
			}
			//else if()
			else if(E_HDR_NORMAL == hdr &&  NULL != pdata + PRO_ADDR_ADD) 
			{
				addr = find_addr(*(pdata + PRO_ADDR_ADD));
				if(E_ADDR_NONE ==  addr)
				{
					Error("not support protocol the addr is %c",*(pdata + PRO_ADDR_ADD));
					res =  false;
					break;
				}
				else if(E_ADDR_NONEED  ==  addr) //地址码
				{			
					if(NULL != pdata + PRO_PROPER_ADD) //协议属性
					{
						proper = find_proper(*(pdata + PRO_PROPER_ADD));
						if(E_PROTO_NONE == proper)
						{
							Error("not support protocol the proporities is %c",*(pdata + PRO_PROPER_ADD));
							res =  false;
							break;
						}	
						else
						{
							if(NULL != pdata + PRO_DEF_ADD) //功能定义
							{
								memset(tempbuf,0,sizeof(tempbuf));
								memcpy(tempbuf,pdata + PRO_DEF_ADD,PRO_TD_ADD-PRO_DEF_ADD);
								def = tempbuf;	

								if(NULL != pdata + PRO_TD_ADD) //通道号
								{
									if (ct_ishex(*(pdata + PRO_TD_ADD))/*ct_isdigit (*(pdata + PRO_TD_ADD))*/)
									{		
										//td = (unsigned char)(*(pdata + PRO_TD_ADD) - '0');
										if (ct_isdigit (*(pdata + PRO_TD_ADD)))
											 td = (unsigned short)(*(pdata + PRO_TD_ADD) - '0');
										else if (ct_islower (*(pdata + PRO_TD_ADD)))
											 td = (unsigned short)(*(pdata + PRO_TD_ADD) - 'a' + 10);
										else
											 td = (unsigned short)(*(pdata + PRO_TD_ADD) - 'A' + 10);
										
										if(NULL != pdata + PRO_DATA_ADD)
										{

											if (E_PROTO_HB == proper ||
											     E_PROTO_ALM  == proper ||
											     //E_PROTO_CFG  == proper ||
											     E_PROTO_CMD  == proper )
											{
												memset(tempbuf,0,sizeof(tempbuf));
												memcpy(tempbuf,pdata + PRO_DATA_ADD,EM_CMD_LEN - PRO_DATA_ADD);
												sdata = tempbuf;
												sdata=trim(sdata);
												//sdata = sdata.trim();
											}
											else if(E_PROTO_DIGIT_ZS == proper ||
												E_PROTO_NEG_DIGIT_ZS  == proper ||
												E_PROTO_DIGIT_XS  == proper ||
												E_PROTO_NEG_DIGIT_XS  == proper ||
												E_PROTO_CFG_ZS == proper ||
												E_PROTO_CFG_XS == proper ||
												E_PROTO_NEG_CFG_ZS == proper ||
												E_PROTO_NEG_CFG_XS == proper)							
											{
												unsigned int    iData1 = 0;
												float    iData2 = 0;
												unsigned char ucTmp1 = '\0';
												unsigned char ucTmp2 = '\0';
												unsigned char ucTmp3 = '\0';
												unsigned char ucTmp4 = '\0';											
												if (NULL != pdata + PRO_DATA_ADD && ct_isdigit (*(pdata + PRO_DATA_ADD)))
												{
													ucTmp1 = (unsigned char)(*(pdata + PRO_DATA_ADD) - '0');													
												}
												if (NULL != pdata + PRO_DATA_ADD + 1 && ct_isdigit (*(pdata + PRO_DATA_ADD + 1)))
												{
													ucTmp2 = (unsigned char)(*(pdata + PRO_DATA_ADD + 1) - '0');
												}
												if (NULL != pdata + PRO_DATA_ADD + 2 && ct_isdigit (*(pdata + PRO_DATA_ADD + 2)))
												{
													ucTmp3 = (unsigned char)(*(pdata + PRO_DATA_ADD + 2) - '0');
												}
												if (NULL != pdata + PRO_DATA_ADD + 3 && ct_isdigit (*(pdata + PRO_DATA_ADD + 3)))
												{
													ucTmp4 = (unsigned char)(*(pdata + PRO_DATA_ADD + 3) - '0');
												}
												//atoi
												if(E_PROTO_DIGIT_ZS == proper || E_PROTO_CFG_ZS == proper)
												{
													iData1 = (unsigned int)ucTmp1 * 1000 + (unsigned int)ucTmp2 * 100 + (unsigned int)ucTmp3 *10 + (unsigned int)ucTmp4;
													data = iData1;
												}
												else if(E_PROTO_NEG_DIGIT_ZS == proper || E_PROTO_NEG_CFG_ZS == proper )
												{
													iData1 = (unsigned int)ucTmp1 * 1000 + (unsigned int)ucTmp2 * 100 + (unsigned int)ucTmp3 *10 + (unsigned int)ucTmp4;
													data = 0 - iData1;
												}
												else if(E_PROTO_DIGIT_XS  == proper || E_PROTO_CFG_XS  == proper)
												{
													iData1 = (unsigned int)ucTmp1 * 10 + (unsigned int)ucTmp2;
													iData2 = (float)ucTmp3 * 0.1 + (float)ucTmp4 * 0.01;
													data = float(iData1) + iData2;												
												}
												else //E_PROTO_NEG_DIGIT_XS
												{
													iData1 = (unsigned int)ucTmp1 * 10 + (unsigned int)ucTmp2;
													iData2 = (float)ucTmp3 * 0.1 + (float)ucTmp4 * 0.01;
													data = 0-(float(iData1) + iData2);															
												}
												
											}
																								
											}
											else
											{
												Error("not support protocol the proper is %c",*(pdata + PRO_PROPER_ADD));
											}
										}
										else
										{
											Error("not support protocol the pointer is null");
											res =  false;	
											break;
										}
										
									}
									else
									{
										Error("tdh is not digit");
										res =  false;
										break;
									}
								}
							}
						}					
						
					}
					else if(E_ADDR_DEVCODE  ==  addr) //'#'
					{
						memset(tempbuf,0,sizeof(tempbuf));
						memcpy(tempbuf,pdata + PRO_PROPER_ADD,EM_CMD_LEN - PRO_PROPER_ADD);
						//sdata = string(tempbuf).trim();	
						sdata = tempbuf;
						sdata=trim(sdata);
					}
					else
					{
						Error("not support protocol the pointer is null");
						res =  false;
						break;
					}				
				}
				else if(E_HDR_REPLY == hdr ) //'@'
				{
					memset(tempbuf,0,sizeof(tempbuf));
					memcpy(tempbuf,pdata + PRO_ADDR_ADD,EM_CMD_LEN - PRO_ADDR_ADD);
					//sdata = string(tempbuf).trim();		
					sdata = tempbuf;
					sdata=trim(sdata);					
				}
				else
				{
					Error("not support protocol the addr is %c",*(pdata + PRO_ADDR_ADD));
					res =  false;
					break;
				}
				
			}
			else
			{
				Error("not support protocol the pointer is null");
				res =  false;
				break;
			}
		
	}while(0);
	return res;
}


bool RspData::saveReqReply(St_ReqReply *pstReqReply)
{
	//save to yw_hfqr
	Info("RspData::saveReqReply");
	char sql[SQLBUFSIZE] = {0,};
	//char tempbuf[EM_CMD_LEN+1] = {0};
	char* pdata = pstReqReply->stReq.recvData;
	time_t t_snd = (time_t)pstReqReply->sndTime.tv_sec;
	time_t t_rsp = (time_t)pstReqReply->rspTime.tv_sec;
	
	//memcpy(tempbuf,pdata,EM_CMD_LEN);
	snprintf( sql, sizeof(sql), "insert into yw_hfqr(sbid,ml,fssj,hfsj) values(%u,'%s',FROM_UNIXTIME(%u),from_unixtime(%u))",
		pstReqReply->devid,pdata,(unsigned int)t_snd,(unsigned int)t_rsp);
	Info("%s",sql);	
	if ( mysql_query( &m_dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
		return false;
	}	
	return true;
}

string RspData::getJqm()
{
	Info("RspData::getJqm");
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	string sbjqm = m_stMsgRsp.tcpSrv->getDevSbJqm();

	#if 1
	if(sbjqm.empty())
	{
		string cmd_devcode = (const char *)config.Item(m_dbconn,E_CFG_D_DEVCODE );
		Error("jqm is null or jqm size != 10");
		//命令主机上报设备全码	
		if(!cmd_devcode.empty() && 0 == (++m_stMsgRsp.tcpSrv->m_devCodetimes) % 2)
		{
			sendCmdJqm(cmd_devcode);
		}
		return "";
	}
	#else
	if(sbjqm.empty())
	{	
		string sbip = m_stMsgRsp.tcpSrv->getDevSbip();
		snprintf( sql, sizeof(sql), "select jqm from zj_table where IPAddress = '%s' and status = 1", sbip.c_str());
		Info("%s",sql);

		if ( mysql_query( &m_dbconn, sql ) )
		{
		     Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
		     return "";
		}
		//get result
		MYSQL_RES *result = mysql_store_result( &m_dbconn );
		if ( !result )
		{
			Error( "Can't use query result: %s", mysql_error( &m_dbconn ) );
			return "";
		}

		int n_items = mysql_num_rows( result );
		if ( n_items != 1 )
		{
			mysql_free_result( result );
			Error( "get rows num is %d", n_items );
			if( 0 == n_items)
			{
				//命令主机上报设备全码			
				sendCmdJqm(cmd_devcode);
				//return "";
			}
			return "";
		}
		//find devid (db)
		MYSQL_ROW dbrow = mysql_fetch_row( result );
		if (mysql_errno( &m_dbconn ))
		{
			mysql_free_result( result );
			Error( "Can't fetch row: %s", mysql_error( &m_dbconn ) );
			return "";
		}
		if (dbrow )
		{
			Info("RspData::getJqm jqm %s",dbrow[0]);
			//return dbrow[0];
		}
		else
		{
			mysql_free_result( result );
			Error( "dbrow is null");
			return "";		
		}
		sbjqm =  dbrow[0];
		mysql_free_result( result );
		if(sbjqm.empty())
		{
			Error("jqm is null");
			//命令主机上报设备全码			
			sendCmdJqm(cmd_devcode);
			return "";
		}
		m_stMsgRsp.tcpSrv->setDevSbJqm(sbjqm);
	}
	#endif
	return sbjqm;

}

unsigned int RspData::getDevId()
{
	Info("RspData::getDevId");
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	string sbjqm = m_stMsgRsp.tcpSrv->getDevSbJqm();

	snprintf( sql, sizeof(sql), "select ifnull(id,0) from zj_table where jqm = '%s' and status = 1", sbjqm.empty()?"":sbjqm.c_str());
	Info("%s",sql);

	if ( mysql_query( &m_dbconn, sql ) )
	{
	     Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
	     return 0;
	}
	//get result
	MYSQL_RES *result = mysql_store_result( &m_dbconn );
	if ( !result )
	{
		Error( "Can't use query result: %s", mysql_error( &m_dbconn ) );
		return 0;
	}

	int n_items = mysql_num_rows( result );
	if ( n_items != 1 )
	{
		mysql_free_result( result );
		Error( "get rows num is %d", n_items );
		return 0;
	}
	//find devid (db)
	MYSQL_ROW dbrow = mysql_fetch_row( result );
	if (mysql_errno( &m_dbconn ))
	{
		mysql_free_result( result );
		Error( "Can't fetch row: %s", mysql_error( &m_dbconn ) );
		return 0;
	}
	if (dbrow )
	{
		Info("RspHB::doProc devid %d",atoi(dbrow[0]));

	}
	else
	{
		mysql_free_result( result );
		Error( "dbrow is null");
		return 0;		
	}
	mysql_free_result( result );
	return atoi(dbrow[0]);
}



bool RspData::sendCmd(string& cmd)
{
	Info("senCmd cmd is %s",cmd.empty()?"":cmd.c_str());
	if(cmd.empty())
	{
		Warning("senCmd cmd is empty");
		return false;
	}
	St_ecdrEncoder * pstEncoder = NULL;
	St_MsgReq stMsgReq = {0,};
	unsigned int  devid = 0;
	//encode request msg
	/*4+1+12*/
	if((devid = getDevId()) == 0)
	{
		return false;
	}
       //encode
       E_CMD_TYPE cmdtype = CMD_ORG;
	pstEncoder = ecdr_CreateEncoder(EM_REQ_LEN);
	ecdr_EncodeUint32(pstEncoder, &devid);
	ecdr_EncodeUint8(pstEncoder, (unsigned char*)&cmdtype);
	ecdr_EncodeOctets(pstEncoder,(char*)cmd.c_str(),EM_CMD_LEN);		
	if(EM_REQ_LEN  != pstEncoder->iPosition)
	{
		Error("RspData::sendCmd encode fail");
		ecdr_DestroyEncoder( pstEncoder );
		return false;
	}
	//send back	
	//stMsgReq.sockAddr = (SockAddr*)(((TcpInetServer*)m_stMsgRsp.tcpSrv)->getLocalAddr())->getTempAddr();
	memcpy(stMsgReq.recvData,pstEncoder->pucData,EM_REQ_LEN);
       if (0 == SendMsgReq(&stMsgReq))
	{
		Error("RspData::sendCmd  sendmsgreq fail");
		ecdr_DestroyEncoder( pstEncoder );
		return false;
	}
	ecdr_DestroyEncoder( pstEncoder );
	return true;
}

bool RspData::sendCmdJqm(string& cmd)
{
	TcpInetSocket *socket = dynamic_cast<TcpInetSocket *>(m_stMsgRsp.tcpSrv); 
	Info("RspData::sendCmdJqm cmd is %s",cmd.c_str());
	if(!socket ||!Inet::sendCmd(socket,cmd))
	{
		Error("RspData::sendCmdJqm  fail");
		return false;	
	}	
	return true;
}

bool RspData::sendReply()
{
	Info("RspData::sendReply");

	char* pdata = m_stMsgRsp.recvData;		
	//unsigned int sendnum = 0;

	char tempbuf[EM_CMD_LEN+1] = {0};
	//reply
	*(pdata + PRO_HDR_ADD) = '@';
	memcpy(tempbuf,pdata,EM_CMD_LEN);
	
	string cmd = tempbuf;
	if(!sendCmd(cmd))
	{
		Error("RspData::sendReply  fail");
		return false;			
	}
	return true;
}

bool RspHB::doProc()
{
	Info("RspHB::doProc");
	char sql[SQLBUFSIZE] = {0,};
	string sbjqm = getJqm();
	if(sbjqm.empty())
	{
		Error("jqm is null");
		return false;
	}
	if(m_def == (const char *)config.Item(m_dbconn,E_CFG_U_HB))
	{
		Info("RspHB::doProc heartbeat ");
		if(m_sdata != sbjqm.substr(6,4))
		{
			Error("RspHB::doProc sbbm not match");
			return false;
		}		
		//refresh device time (db)	
		//网络状态
		if(m_td != 0)
		{
			//此种情况目前没有
			snprintf( sql, sizeof(sql), "update  sb_table set IPAddress ='%s',updateDate= DATE_FORMAT(now(), '%%Y-%%m-%%d %%H:%%i:%%s') where zjid in (select id from zj_table where jqm = '%s' and status = 1) and zjtd=%d and status = 1", m_stMsgRsp.tcpSrv->getDevSbip().c_str(),sbjqm.c_str(),m_td);
		}
		else
		{
			snprintf( sql, sizeof(sql), "update  zj_table set IPAddress ='%s',updateDate= DATE_FORMAT(now(), '%%Y-%%m-%%d %%H:%%i:%%s'), wlzt = 1 where jqm = '%s' and status = 1", m_stMsgRsp.tcpSrv->getDevSbip().c_str(),sbjqm.c_str());
		}
		Info("%s",sql);
		if ( mysql_query( &m_dbconn, sql ) )
		{
			Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
			return false;
		}
	}
	else if(m_def =="ZtBg")
	{
		Info("RspHB::doProc status");
		return true;
	}	
	else
	{
		Info("RspHB::doProc unsupport");
		return false;
	}
	//store db
	CommRec commRec(m_dbconn,m_stMsgRsp,m_hdr,m_addr,m_proper,m_def,m_td,m_orgcmd,m_sdata);
	int rowid = 0;
	if(!commRec.saveTodb(rowid))
	{
		Error("RspHB::doProc saveTodb fail");
		return false;
	}
	//reply
	if(!sendReply())
	{
		Error("RspHB::doProc sendReply fail");
		return false;
	}
	return true;
	
}

bool RspAlm::doProc()
{
	Info("RspAlm::doProc");
	St_UnsolFEndMsg stUnFEndMsg;
	char sql[SQLBUFSIZE] = {0,};
	string sbjqm = getJqm();
	if(sbjqm.empty())
	{
		Error("jqm is null");
		return false;
	}
	//save to db
	AlmRec almRec(m_dbconn,m_stMsgRsp,m_hdr,m_addr,m_proper,m_def,m_td,m_orgcmd,m_sdata);
	int rowid = 0;
	if(!almRec.saveTodb(rowid))
	{
		Error("RspAlm::doProc saveTodb fail");
		return false;
	}
	//reply
	if(!sendReply())
	{
		Error("RspAlm::doProc sendReply fail");
		return false;
	}	
	//update device status
	E_ALM_LVL jqjb = E_ALM_UNKNOWN;
	//get jqjb
	if(m_sdata == "JinB" || m_sdata == "alm1" ||m_sdata=="alam")
	{
		jqjb = E_ALM_JB;
	}
	else if(m_sdata == "JinS" || m_sdata == "alm2" ||m_sdata=="warn")
	{
		jqjb = E_ALM_JS;
	}
	else if(m_sdata == "Ycan" || m_sdata == "alm3" ||m_sdata=="abno")
	{
		jqjb = E_ALM_YC;
	}
	else
	{
		jqjb = E_ALM_UNKNOWN;
	}
	
	if(m_td != 0)
	{
		snprintf( sql, sizeof(sql), "update  sb_table set updateDate= DATE_FORMAT(now(), '%%Y-%%m-%%d %%H:%%i:%%s'),gzjb=%u,gjcs=gjcs+1 where zjid in (select id from zj_table where jqm = '%s' and status = 1) and zjtd=%d and status = 1",jqjb,sbjqm.c_str(),m_td);
	}
	else
	{	// 信号灯故障???
		//主机警报
		snprintf( sql, sizeof(sql), "update  zj_table set updateDate= DATE_FORMAT(now(), '%%Y-%%m-%%d %%H:%%i:%%s'), wlzt = 1 where jqm = '%s' and status = 1", sbjqm.c_str());
	}
	Info("%s",sql);
	if ( mysql_query( &m_dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
		return false;
	}	
	
	//alarm fend
	#if 0
	stUnFEndMsg.msgtype =E_ALARM;
	stUnFEndMsg.jlid = rowid;
	if(0 == unsFEndMsg(stUnFEndMsg))
	{
		return false;
	}
	#endif
	return true;
}

bool RspCfg::doProc()
{
	Info("RspCfg::doProc");
	string sbjqm = getJqm();
	if(sbjqm.empty())
	{
		Error("jqm is null");
		return false;
	}
	char* pdata = m_stMsgRsp.recvData;	
       if(0 == strncmp(pdata,(const char *)config.Item(m_dbconn,E_CFG_U_ZMDBZ),7))
   	{
   		//照明灯上报标准
   		char sql[SQLBUFSIZE] = {0,};
   		Info("RspCfg::doProc zmd standard");
		if(m_td != 0)
		{
			snprintf( sql, sizeof(sql), "update  sb_table set bz = %f,updateDate = DATE_FORMAT(now(), '%%Y-%%m-%%d %%H:%%i:%%s') where zjid in (select id from zj_table where jqm = '%s' and status = 1) and zjtd=%d and status = 1", m_data,sbjqm.empty()?"":sbjqm.c_str(),m_td);
			Info("%s",sql);
			if ( mysql_query( &m_dbconn, sql ) )
			{
				Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
				return false;
			}
		}
   		
   	}
	
	//save to db
	CommRec commRec(m_dbconn,m_stMsgRsp,m_hdr,m_addr,m_proper,m_def,m_td,m_orgcmd,m_data);
	int rowid = 0;
	if(!commRec.saveTodb(rowid))
	{
		Error("RspDevCode::doProc saveTodb fail");
		return false;
	}	
	return true;
}

bool RspDevCode::doProc()
{
	Info("RspDevCode::doProc");
	char sql[SQLBUFSIZE] = {0,};
	if(m_sdata.empty()  || trim(m_sdata).size() != 10)  return false;
	//set jqm
	m_stMsgRsp.tcpSrv->setDevSbJqm(m_sdata);
	//refresh ip
	snprintf( sql, sizeof(sql), "update  zj_table set IPAddress ='%s',updateDate = DATE_FORMAT(now(), '%%Y-%%m-%%d %%H:%%i:%%s'), wlzt = 1 where jqm = '%s' and status = 1", m_stMsgRsp.tcpSrv->getDevSbip().c_str(),m_sdata.c_str());
	Info("%s",sql);
	if ( mysql_real_query(&m_dbconn, sql,strlen(sql)))
	{
		Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	Info("RspDevCode::doProc update rows is %u",mysql_affected_rows(&m_dbconn));
	//if not found jqm,new device come in ??
	if (mysql_affected_rows(&m_dbconn) < 1)
	{
		//new device
		memset(sql,0,sizeof(sql));
		snprintf(sql, sizeof(sql), "insert into  zj_table(name,IPAddress,jqm,updateDate,status,wlzt) values('new device','%s','%s',DATE_FORMAT(now(), '%%Y-%%m-%%d %%H:%%i:%%s'),1,1)", m_stMsgRsp.tcpSrv->getDevSbip().c_str(),m_sdata.c_str());
		Info("new device  and sql :%s",sql);
		if ( mysql_query( &m_dbconn, sql ) )
		{
			Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
			return false;
		}
		int rowid = mysql_insert_id(&m_dbconn);
		//unsolicited fend
		St_UnsolFEndMsg stUnFEndMsg;
		stUnFEndMsg.msgtype =E_NCDEV;
		stUnFEndMsg.jlid = rowid;
		/*
		if(0 == unsFEndMsg(stUnFEndMsg))
		{
			return false;
		}
		*/
	}
	
	//save to db
	CommRec commRec(m_dbconn,m_stMsgRsp,m_hdr,m_addr,m_proper,m_def,m_td,m_orgcmd,m_sdata);
	int rowid = 0;
	if(!commRec.saveTodb(rowid))
	{
		Error("RspDevCode::doProc saveTodb fail");
		return false;
	}
	//no reply
	return true;
}


bool RspReply::doProc()
{
	Info("RspReply::doProc");
	St_RspFEndMsg stRspFEndMsg;
	char* pRspdata = m_stMsgRsp.recvData;
	ReqList reqlist = MsgReqThread::m_reqCmdList;
	string sbjqm = getJqm();
	if(sbjqm.empty())
	{
		Error("jqm is null");
		return false;
	}
#ifdef REQ_REPLY	
	if (reqlist.size() == 0)
	{
		Error("null request list");
		//return false;
	}
	else
	{
		bool found = false;
		int devid = getDevId();
		for(ReqList ::iterator iter = reqlist.begin(); iter != reqlist.end();)  
		{  
			St_ReqReply * pReqReply = (St_ReqReply*)(*iter);
			char* pReqdata =  pReqReply ->stReq.recvData;
			if(pReqReply ->devid == devid  &&  0 == strncmp(pReqdata+ PRO_ADDR_ADD,pRspdata + PRO_ADDR_ADD,EM_CMD_LEN - PRO_ADDR_ADD))
			{
				Info("RspReply::doProc found request item");	
				pReqReply->rspTime = tvNow();
				//yw_hfqr
				if(!saveReqReply(pReqReply))
				{		
					//reqlist.erase(iter);
					//delete(*iter);
					//return false;
				}
				iter = reqlist.erase(iter);
				delete(pReqReply);
				found = true;
				break;
			}
			else
			{
				char ReqTemp[EM_CMD_LEN+1] = {0,};
				char RspTemp[EM_CMD_LEN+1] = {0,};
				memcpy(ReqTemp,pReqdata,EM_CMD_LEN);
				memcpy(RspTemp,pRspdata,EM_CMD_LEN);	
				Debug(2,"RspReply::doProc reqdata is %s,rspdata is %s",ReqTemp,RspTemp);
				iter++;
			}
		}  
		if(!found)
		{
			Error("not found request list");
			//return false;
		}
	}
#endif
	//save to db
	CommRec commRec(m_dbconn,m_stMsgRsp,m_hdr,m_addr,m_proper,m_def,m_td,m_orgcmd,m_sdata);
	int rowid = 0;
	if(!commRec.saveTodb(rowid))
	{
		Error("RspReply::doProc saveTodb fail");
		return false;
	}	
	//fend
	stRspFEndMsg.restype = E_SUC;
	stRspFEndMsg.txsjid = rowid;
	//if(0 == rspFEndMsg(stRspFEndMsg))
	{
	//	return false;
	}
	return true;
}

bool RspDigit::doProc()
{
	Info("RspDigit::doProc");
	string sbjqm = getJqm();
	char sql[SQLBUFSIZE] = {0,};
	if(sbjqm.empty())
	{
		Error("jqm is null");
		return false;
	}
	
	snprintf( sql, sizeof(sql), "update  zj_table set IPAddress ='%s',updateDate= DATE_FORMAT(now(), '%%Y-%%m-%%d %%H:%%i:%%s'), wlzt = 1 where jqm = '%s' and status = 1", m_stMsgRsp.tcpSrv->getDevSbip().c_str(),sbjqm.c_str());	
	Info("%s",sql);
	if (mysql_query( &m_dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	
	//save to db
	CommRec commRec(m_dbconn,m_stMsgRsp,m_hdr,m_addr,m_proper,m_def,m_td,m_orgcmd,m_data);
	int rowid = 0;
	if(!commRec.saveTodb(rowid))
	{
		Error("RspDigit::doProc saveTodb fail");
		return false;
	}

	return true;
}

