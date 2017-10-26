

#include "em.h"
#include "logger.h"
#include "msgqueue.h"
#include "msghdl.h"

#include "reqdata.h"
#include "rspdata.h"
#include "comms.h"
#include "commrec.h"

//E_GRP_FLAG CommRec::m_seGrplast = E_GRP_NONE;
bool CommRec::saveTodb(int &rowid)
{
	Info("CommRec::saveTodb");

	if (E_HDR_REPLY == m_hdr)
	{
		m_def = "@";
		m_addr = E_ADDR_NONE;
		m_proper = E_PROTO_NONE;
		m_td = 0;
		m_xmmc = "reply";

		m_xmid = 0;
		m_groupid = 0;
		m_sblx= 0;
	}
	else if (E_ADDR_DEVCODE == m_addr)
	{
		m_def = "#";
		m_proper = E_PROTO_NONE;
		m_td = 0;
		m_xmmc = "device code";
		
		m_xmid = 0;
		m_groupid = 0;
		m_sblx = 0;

	}
	else if(E_ADDR_NONEED == m_addr)
	{
		if(!getXmDetl())
		{
			Error("CommRec::saveTodb getXmDetl fail");
			return false;
		}			
		
	}
	else
	{
		Error("do nothing");
		return false;
	}
	if(!getDevInfo())
	{
		Error("CommRec::saveTodb getDevInfo fail");
		return false;
	}
	char sql[SQLBUFSIZE] = {0,};
	if(	m_def == "@" ||
		m_def =="#" ||
		//E_ADDR_DEVCODE == m_addr ||
		E_PROTO_HB == m_proper ||
		//E_PROTO_ALM == m_proper ||
		//E_PROTO_CFG == m_proper ||
		E_PROTO_CMD == m_proper)
	{
		
		snprintf(sql, sizeof(sql), "insert into yw_txsj(sbid,sbmc,jqm,sblx,ip,fx,tdh,zh,ssid,ssmc,ssfz,sszl,xmid,xmbm,xmmc,xysx,xylb,sjzzf,orgcmd,txsj)  values(%u,'%s','%s',%u,'%s','%s',%u,%u,%u,'%s',%u,%u,%u,'%s','%s',%u,%u,'%s','%s',now())",
			m_sbid,m_sbmc.empty()?"":m_sbmc.c_str(),m_jqm.empty()?"":m_jqm.c_str(),m_sblx,m_ip.empty()?"":m_ip.c_str(),m_fx.empty()?"":m_fx.c_str(),m_td,m_groupid,m_ssid,m_ssmc.empty()?"":m_ssmc.c_str(),m_ssfz,
			m_sszl,m_xmid,m_def.empty()?"":m_def.c_str(),m_xmmc.empty()?"":m_xmmc.c_str(),m_proper,m_hdr,m_sdata.empty()?"":m_sdata.c_str(),m_orgcmd.empty()?"":m_orgcmd.c_str());
		
	}
	else if(E_PROTO_DIGIT_ZS == m_proper ||
		E_PROTO_DIGIT_XS == m_proper ||
		E_PROTO_NEG_DIGIT_ZS == m_proper ||
		E_PROTO_NEG_DIGIT_XS == m_proper ||
		E_PROTO_CFG_ZS == m_proper ||
		E_PROTO_CFG_XS == m_proper ||
		E_PROTO_NEG_CFG_ZS == m_proper ||
		E_PROTO_NEG_CFG_XS == m_proper)
	{
	
		 m_seGrplast= m_stMsgRsp.tcpSrv->getDevGrpLast();
	
		//是否分组?
		if(E_GRP_ED == m_seGrplast)
		{
			Error("group latest is E_GRP_ED");
			m_seGrplast = E_GRP_NONE;
			m_stMsgRsp.tcpSrv->setDevGrpLast(m_seGrplast);
			return false;			
		}	
		else if(E_GRP_ST == m_seGrplast) //组已开始
		{
			if(E_GRP_ST == m_seGrp)
			{
				Error("abnormal group:not found  end of group and next start point come in. discard");		
				m_seGrplast = E_GRP_NONE;
				m_stMsgRsp.tcpSrv->setDevGrpLast(m_seGrplast);
				return false;
			}
			else if(E_GRP_MID == m_seGrp) //找到最近同设备组号zh
			{
				m_groupid = getGroupID();
				if(0 == m_groupid)
				{
					m_seGrplast = E_GRP_NONE;
					Error("abnormal group:not found group id 1");
					m_stMsgRsp.tcpSrv->setDevGrpLast(m_seGrplast);
					return false;
				}
				m_seGrplast = E_GRP_MID;
				
			}
			else if(E_GRP_NONE == m_seGrp)  //非分组项目
			{
				m_groupid = 0;
				
			}
			else //E_GRP_ED  分组结束
			{
				m_groupid = getGroupID();
				if(0 == m_groupid)
				{
					m_seGrplast = E_GRP_NONE;
					Error("abnormal group:not found group id 2");
					m_stMsgRsp.tcpSrv->setDevGrpLast(m_seGrplast);
					return false;
				}
				m_seGrplast = E_GRP_NONE;
			}
		}
		else if(E_GRP_MID == m_seGrplast) //组已有子项
		{
			if(E_GRP_ST == m_seGrp)
			{
				Error("abnormal group:not end. discard");
				m_seGrplast = E_GRP_NONE;
				m_stMsgRsp.tcpSrv->setDevGrpLast(m_seGrplast);
				return false;				
			}
			else if(E_GRP_MID == m_seGrp) //分组开始
			{
				m_groupid = getGroupID();
				m_seGrplast = E_GRP_MID;
			}
			else if(E_GRP_ED == m_seGrp)
			{
				m_groupid = getGroupID();
				m_seGrplast = E_GRP_NONE;		
			}			
			else //E_GRP_NONE 非分组项目
			{
				m_groupid = 0;
			}			
		}
		else //E_GRP_NONE
		{
			if(E_GRP_ED == m_seGrp)
			{
				Error("abnormal group:first group is E_GRP_ED. discard");
				m_seGrplast = E_GRP_NONE;
				m_stMsgRsp.tcpSrv->setDevGrpLast(m_seGrplast);
				return false;				
			}
			else if(E_GRP_ST == m_seGrp) //分组开始
			{
				m_groupid = getNextGroupID();
				m_seGrplast = E_GRP_ST;
			}
			else if(E_GRP_MID == m_seGrp) //分组开始
			{
				Error("abnormal group:not start. discard");
				m_seGrplast = E_GRP_NONE;
				m_stMsgRsp.tcpSrv->setDevGrpLast(m_seGrplast);
				return false;	
			}
			else //E_GRP_NONE 非分组项目
			{
				m_groupid = 0;
			}
				
		}	

		m_stMsgRsp.tcpSrv->setDevGrpLast(m_seGrplast);
		//
		snprintf( sql, sizeof(sql), "insert into yw_txsj(sbid,sbmc,jqm,sblx,ip,fx,tdh,zh,ssid,ssmc,ssfz,sszl,xmid,xmbm,xmmc,sjl,xysx,xylb,sjz,orgcmd,txsj) values(%u,'%s','%s',%u,'%s','%s','%u',%u,%u,'%s',%u,%u,%u,'%s','%s',%u,%u,%u,%f,'%s',now())",
			m_sbid,m_sbmc.empty()?"":m_sbmc.c_str(),m_jqm.empty()?"":m_jqm.c_str(),m_sblx,m_ip.empty()?"":m_ip.c_str(),m_fx.empty()?"":m_fx.c_str(),m_td,m_groupid,m_ssid,m_ssmc.empty()?"":m_ssmc.c_str(),m_ssfz,
			m_sszl,m_xmid,m_def.empty()?"":m_def.c_str(),m_xmmc.empty()?"":m_xmmc.c_str(),m_dtclass,m_proper,m_hdr,m_data,m_orgcmd.empty()?"":m_orgcmd.c_str());
		
	}
	Info("%s",sql);	
	if (mysql_query( &m_dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	rowid = mysql_insert_id(&m_dbconn);
	return true;
}

unsigned int CommRec::getNextGroupID()
{
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	unsigned int groupid = 0;
	Info("CommRec::getMaxGroupID");

	snprintf( sql, sizeof(sql), "select ifnull(max(zh),0) from yw_txsj where sbid = %u and xysx in(%u,%u,%u,%u) and sjl = %u", m_sbid,E_PROTO_DIGIT_ZS,E_PROTO_DIGIT_XS,E_PROTO_NEG_DIGIT_ZS,E_PROTO_NEG_DIGIT_XS,m_dtclass);
	
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

	MYSQL_ROW dbrow = mysql_fetch_row( result );
	//for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	if (mysql_errno( &m_dbconn ))
	{
		mysql_free_result( result );
		Error( "Can't fetch row: %s", mysql_error( &m_dbconn ) );
		return 0;
	}
	if (dbrow)
	{
		groupid = atoi(dbrow[0]);

	}
	else
	{
		mysql_free_result( result );
		Error( "dbrow is null");
		return 0;		
	}
	mysql_free_result( result );

	Info("next group id=%u", ++groupid);
	return groupid;	
}

unsigned int CommRec::getGroupID()
{
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	unsigned int groupid = 0;
	string xmbm;
	Info("CommRec::getGroupID sjl=%d,fzbz=%d",int(m_dtclass),int(E_GRP_ED));

	//获取组结束功能码
	//snprintf( sql, sizeof(sql), "select xmbm from yw_xm where lb =%d and sjl = %d and fzbz = %d",m_sblx,int(m_dtclass),int(E_GRP_ED));	
	snprintf( sql, sizeof(sql), "select ifnull(xmbm,'') from yw_xm where sjl = %d and fzbz = %d",int(m_dtclass),int(E_GRP_ED));	
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
	MYSQL_ROW dbrow = mysql_fetch_row( result );
	//for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	if (mysql_errno( &m_dbconn ))
	{
		mysql_free_result( result );
		Error( "Can't fetch row: %s", mysql_error( &m_dbconn ) );
		return 0;
	}
	if (dbrow)
	{
		xmbm = dbrow[0];
		Info("CommRec::getGroupID xmbm=%s",xmbm.empty()?"":xmbm.c_str());

	}
	else
	{
		mysql_free_result( result );
		Error( "dbrow is null");
		return 0;		
	}
	mysql_free_result( result );	


	//获取组没结束的组号
	snprintf( sql, sizeof(sql), "select  ifnull(max(zh),0) from yw_txsj a where sbid = %u and sjl = %d and zh not in (SELECT zh from yw_txsj where xmbm = '%s' and sbid = %u and sjl = %d)", 
			m_sbid,m_dtclass,xmbm.empty()?"":xmbm.c_str(),m_sbid,m_dtclass);
	
	Info("%s",sql);	

	if ( mysql_query( &m_dbconn, sql ) )
	{
	     Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
	     return 0;
	}
	//get result
	result = mysql_store_result( &m_dbconn );
	if ( !result )
	{
		Error( "Can't use query result: %s", mysql_error( &m_dbconn ) );
		return 0;
	}
	n_items = mysql_num_rows( result );
	if ( n_items != 1 )
	{
		mysql_free_result( result );
		Error( "get rows num is %d", n_items );
		return 0;
	}
	dbrow = mysql_fetch_row( result );
	//for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	if (mysql_errno( &m_dbconn ))
	{
		mysql_free_result( result );
		Error( "Can't fetch row: %s", mysql_error( &m_dbconn ) );
		return 0;
	}
	if (dbrow)
	{
		groupid = atoi(dbrow[0]);
	}
	else
	{
		mysql_free_result( result );
		Error( "dbrow is null");
		return 0;		
	}
	mysql_free_result( result );

	Info("group id=%u", groupid);
	return groupid;	
}

bool CommRec::getXmDetl()
{
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	Info("CommRec::getXmDetl");
	snprintf( sql, sizeof(sql), "select ifnull(xmid,0),ifnull(xmmc,''),ifnull(lb,0),ifnull(sx,0),ifnull(xx,0),ifnull(jqlb,0),ifnull(fzbz,0),ifnull(sjl,0) from yw_xm where xmbm = '%s' and xysx = %u and zt = 1", m_def.empty()?"":m_def.c_str(),m_proper);
	Info("%s",sql);	
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
	if (mysql_errno( &m_dbconn ))
	{
		mysql_free_result( result );
		Error( "Can't fetch row: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	if (dbrow)
	{
		int col = 0;
		m_xmid = atoi(dbrow[col++]);
		m_xmmc = dbrow[col++];
		if(m_td == 0)
		{
			//m_sblx = atoi(dbrow[col++]);
			col++;
		}
		else
		{
			col++;
		}
		m_sx = atof(dbrow[col++]);
		m_xx = atof(dbrow[col++]);
		m_jqlb = atoi(dbrow[col++]);
		m_seGrp = (E_GRP_FLAG)atoi(dbrow[col++]);
		m_dtclass = (E_DATA_CLASS)atoi(dbrow[col++]);
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
bool CommRec::getDevInfo()
{
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	Info("CommRec::getDevInfo");
	if(m_td != 0)
	{
		snprintf( sql, sizeof(sql), "select ifnull(a.id,0),ifnull(a.name,''),ifnull(b.lxid,0),'','',ifnull(ssid,0),ifnull(ssfz,0),ifnull(sscl,0) from sb_table a,sb_type_table b  where a.sblx = b.id and zjid in (select id from zj_table where jqm = '%s' and status = 1) and zjtd=%d and status = 1", m_jqm.empty()?"":m_jqm.c_str(),m_td);
	
	}
	else
	{
		//snprintf( sql, sizeof(sql), "select a.id,a.name,ifnull(b.lxid,0),IPAddress,'',ifnull(ssid,0),ifnull(ssfz,0),ifnull(sszl,0) from zj_table a,sb_type_table b where a.sblx = b.id and jqm = '%s' and status = 1", m_jqm.c_str());
		snprintf(sql, sizeof(sql),"select ifnull(a.id,0),ifnull(a.name,''),ifnull(c.lxid,0),ifnull(a.IPAddress,''),'',ifnull(a.ssid,0),ifnull(a.ssfz,0),ifnull(a.sszl,0) from zj_table a LEFT JOIN sb_table b on a.id = b.zjid INNER JOIN sb_type_table c on c.id =  ifnull(b.sblx,a.sblx) where a.jqm = '%s' and a.status = 1 limit 1" ,m_jqm.empty()?"":m_jqm.c_str());
	}
	Info("%s",sql);	

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
		int col = 0;
		m_sbid = atoi(dbrow[col++]);
		m_sbmc = dbrow[col++];
		/*
		if(m_td != 0)
		{
			m_sblx = atoi(dbrow[col++]); //atoi(dbrow[col++]);
		}
		else
		{
			col++;
		}
		*/
		m_sblx = atoi(dbrow[col++]);
		
		m_ip = dbrow[col++];
		m_fx = dbrow[col++];
		m_ssid = atoi(dbrow[col++]);
		//m_ssmc = dbrow[col++];
		m_ssfz = atoi(dbrow[col++]);
		m_sszl = atoi(dbrow[col++]);
		//Info("CommRec::getDevInfo sbid %u,sbmc %s,sblx %u,sblxmc %s,ip %s,fx %s,ssid %u,ssfz %u,sszl %u",
		//	m_sbid,m_sbmc.c_str(),m_sblx,m_ip.c_str(),m_fx.c_str(),m_ssid,m_ssfz,m_sszl);
	}
	else
	{
		mysql_free_result( result );
		Error( "dbrow is null");
		return false;		
	}
	mysql_free_result( result );
	if(!getSSInfo())
	{
		Error("CommRec::getDevInfo getssinfo fail");
		//return false;
	}
	return true;	
}

bool CommRec::getSSInfo()
{
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	//string	m_ssmc;
	//select * from ss

	Info("CommRec::getSSInfo");

	//获取设施信息
	//and sbzt = 1
	snprintf( sql, sizeof(sql), "select ifnull(name,''),ifnull(code,''),ifnull(sslx,0) from ss_table where id = %u", m_ssid);
	
	Info("%s",sql);	

	if ( mysql_query( &m_dbconn, sql))
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
		int col = 0;
		m_ssmc = dbrow[col++];
		m_ssbm = dbrow[col++];
		m_sslx = atoi(dbrow[col++]);
		Info("ssmc is %s",m_ssmc.empty()?"":m_ssmc.c_str());
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


bool AlmRec::saveTodb(int &rowid)
{
	Info("AlmRec::saveTodb sdata=%s",m_sdata.empty()?"":m_sdata.c_str());
	char sql[SQLBUFSIZE] = {0,};

	if(!getXmDetl())
	{
		Error("AlmRec::saveTodb getXmDetl fail");
		return false;
	}

	if(!getDevInfo())
	{
		Error("AlmRec::saveTodb getDevInfo fail");
		return false;
	}
	//get jqjb
	if(m_sdata == "JinB" || m_sdata == "alm1" ||m_sdata=="alam")
	{
		m_jqjb = E_ALM_JB;
	}
	else if(m_sdata == "JinS" || m_sdata == "alm2" ||m_sdata=="warn")
	{
		m_jqjb = E_ALM_JS;
	}
	else if(m_sdata == "Ycan" || m_sdata == "alm3" ||m_sdata=="abno")
	{
		m_jqjb = E_ALM_YC;
	}
	else
	{
		m_jqjb = E_ALM_UNKNOWN;
	}

	snprintf( sql, sizeof(sql), "insert into yw_txsj(sbid,sbmc,jqm,sblx,ip,fx,tdh,zh,ssid,ssmc,ssfz,sszl,xmid,xmbm,xmmc,xysx,xylb,sjzzf,orgcmd,txsj)  values(%u,'%s','%s',%u,'%s','%s',%u,%u,%u,'%s',%u,%u,%u,'%s','%s',%u,%u,'%s','%s',now())",
		m_sbid,m_sbmc.empty()?"":m_sbmc.c_str(),m_jqm.empty()?"":m_jqm.c_str(),m_sblx,m_ip.empty()?"":m_ip.c_str(),m_fx.empty()?"":m_fx.c_str(),m_td,m_groupid,m_ssid,m_ssmc.empty()?"":m_ssmc.c_str(),m_ssfz,
		m_sszl,m_xmid,m_def.empty()?"":m_def.c_str(),m_xmmc.empty()?"":m_xmmc.c_str(),m_proper,m_hdr,m_sdata.empty()?"":m_sdata.c_str(),m_orgcmd.empty()?"":m_orgcmd.c_str());
	Info("%s",sql);
	if ( mysql_query( &m_dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	
	//sjz -->string
	snprintf( sql, sizeof(sql), "insert into yw_jqgl(sbid,sbmc,jqm,sblx,ip,fx, tdh,ssid,sscode,ssmc,sslx,ssfz,sscl,xmid,xmbm,xmmc,sjz,orgcmd,sx,xx,jqlb,jqjb,jqsj) values(%u,'%s','%s',%u,'%s','%s',%u,%u,'%s','%s',%u,%u,%u,%u,'%s','%s','%s','%s',%f,%f,%u,%u,now())",
	m_sbid,m_sbmc.empty()?"":m_sbmc.c_str(),m_jqm.empty()?"":m_jqm.c_str(),m_sblx,m_ip.empty()?"":m_ip.c_str(),m_fx.empty()?"":m_fx.c_str(),m_td,m_ssid,m_ssbm.empty()?"":m_ssbm.c_str(),m_ssmc.empty()?"":m_ssmc.c_str(),m_sslx,m_ssfz,
	m_sszl,m_xmid,m_def.empty()?"":m_def.c_str(),m_xmmc.empty()?"":m_xmmc.c_str(),m_sdata.empty()?"":m_sdata.c_str(),m_orgcmd.empty()?"":m_orgcmd.c_str(),m_sx,m_xx,m_jqlb,m_jqjb);

	Info("%s",sql);	
	if ( mysql_query( &m_dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
		return false;
	}
	rowid = mysql_insert_id(&m_dbconn);

	
	return true;

}

bool OperationRec::saveTodb(int &rowid)
{
	Info("OperationRec::saveTodb");
	if(CMD_ORG == m_ecmdtype)
	{
		if(!getDevInfo())
		{
			Error("OperationRec::saveTodb getdevinfo fail");
			return false;
		}
		else
		{
			char sql[SQLBUFSIZE] = {0,};
			snprintf( sql, sizeof(sql), "insert into yw_czsj(sbid,sbmc,jqm,sblx,ip,ssid,ssmc,ssfz,sszl,xmbm,czsj) values(%u,'%s','%s', %u,'%s',%u,'%s',%u,%u,'%s',now())",
				m_sbid,m_sbmc.empty()?"":m_sbmc.c_str(),m_jqm.empty()?"":m_jqm.c_str(),m_sblx,m_ip.empty()?"":m_ip.c_str(),m_ssid,m_ssmc.empty()?"":m_ssmc.c_str(),m_ssfz,
				m_sszl,m_xmbm.empty()?"":m_xmbm.c_str());
			Info("%s",sql);	
			if ( mysql_query( &m_dbconn,sql) )
			{
				Error( "Can't run query: %s", mysql_error( &m_dbconn ) );
				return false;
			}
		}
		
	}
	else if(CMD_DIGIT == m_ecmdtype)
	{
	}
	else
	{
		Error("not support cmd type");
		return false;
	}
	return true;
}

bool OperationRec::getDevInfo()
{
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);

	Info("OperationRec::getDevInfo");

	//获取主机信息
	snprintf( sql, sizeof(sql), "select ifnull(a.name,''),ifnull(jqm,''),ifnull(b.lxid,0),ifnull(IPAddress,''),ifnull(ssid,0), ifnull(ssfz,0),ifnull(sszl,0) from zj_table a,sb_type_table b  where a.sblx = b.id and  a.id = %u and status = 1", m_sbid);
	
	Info("%s",sql);	

	if ( mysql_query( &m_dbconn, sql))
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
		int col = 0;
		m_sbmc = dbrow[col++];
		m_jqm = dbrow[col++];
		m_sblx = atoi(dbrow[col++]);
		m_ip = dbrow[col++];
		m_ssid = atoi(dbrow[col++]);
		m_ssfz = atoi(dbrow[col++]);
		m_sszl = atoi(dbrow[col++]);
		
		Info("CommRec::getDevInfo sbid %u,sbmc %s,sblx %u,ip %s,fx %s,ssid %u,ssmc %s,ssfz %u,sszl %u",
			m_sbid,m_sbmc.empty()?"":m_sbmc.c_str(),m_sblx,m_ip.empty()?"":m_ip.c_str(),m_fx.empty()?"":m_fx.c_str(),m_ssid,m_ssmc.empty()?"":m_ssmc.c_str(),m_ssfz,m_sszl);
		
	}
	else
	{
		mysql_free_result( result );
		Error( "dbrow is null");
		return false;		
	}
	mysql_free_result( result );
	
	if(!getSSInfo())
	{
		Error("OperationRec::getDevInfo getssinfo fail");
		//return false;
	}
	return true;
	
}


bool OperationRec::getSSInfo()
{
	char sql[SQLBUFSIZE] = {0,};
	memset(sql,0,SQLBUFSIZE);
	//string	m_ssmc;
	//select * from ss

	Info("OperationRec::getSSInfo");

	//获取主机信息
	snprintf( sql, sizeof(sql), "select ifnull(name,'') from ss_table where id = %u", m_ssid);
	
	Info("%s",sql);	

	if ( mysql_query( &m_dbconn, sql))
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
		m_ssmc = dbrow[0];
		Info("ssmc is %s",m_ssmc.empty()?"":m_ssmc.c_str());
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