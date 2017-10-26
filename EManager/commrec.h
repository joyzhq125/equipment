#ifndef COMMREC_H
#define COMMREC_H

/*
typedef enum
{
	E_GRP_NONE = 0x00,
	E_GRP_ST, //分组开始
	E_GRP_MID,  //分组子项
	E_GRP_ED //分组结束
}E_GRP_FLAG;
*/
//车行分段数据记录表
//上传数据类型
typedef enum
{
	E_DATA_CLASS_NONE = 0x00,
	E_DATA_CLASS_NORMAL,	//普通类
	E_DATA_CLASS_XHD_1, //周期时长记录表
	E_DATA_CLASS_XHD_2, //东西人行绿分周
	E_DATA_CLASS_XHD_3, //东西人行红分周
	E_DATA_CLASS_XHD_4, //南北人行绿分周
	E_DATA_CLASS_XHD_5, //南北人行红分周	
	E_DATA_CLASS_RESERVE1,
	E_DATA_CLASS_RESERVE2,
	E_DATA_CLASS_RESERVE3,
	E_DATA_CLASS_RESERVE4,
	E_DATA_CLASS_RESERVE5,
	E_DATA_CLASS_RESERVE6,
	E_DATA_CLASS_RESERVE7,
	E_DATA_CLASS_RESERVE8
}E_DATA_CLASS;

typedef enum
{
	E_ALM_JB = 0x01, //警报
	E_ALM_JS, //警示
	E_ALM_YC, //异常
	E_ALM_UNKNOWN
}E_ALM_LVL;

class CommRec
{
public:
	CommRec(MYSQL &dbconn,St_MsgRsp &stMsgRsp,E_PROTO_HDR hdr,
		E_PROTO_ADDR addr,E_PROTO_PROPER proper,string& def,
		unsigned short td,string& orgcmd,string sdata):m_stMsgRsp(stMsgRsp),m_dbconn(dbconn),m_hdr(hdr),m_addr(addr),m_proper(proper),m_def(def),m_td(td),m_sdata(sdata),m_orgcmd(orgcmd)
	{
		 m_jqm= m_stMsgRsp.tcpSrv->getDevSbJqm();
	}
	CommRec(MYSQL &dbconn,St_MsgRsp &stMsgRsp,E_PROTO_HDR hdr,
		E_PROTO_ADDR addr ,E_PROTO_PROPER proper,string& def,
		unsigned short td,string& orgcmd,float data):m_stMsgRsp(stMsgRsp),m_dbconn(dbconn),m_hdr(hdr),m_addr(addr),m_proper(proper),m_def(def),m_td(td),m_data(data),m_orgcmd(orgcmd)
	{
		 m_jqm= m_stMsgRsp.tcpSrv->getDevSbJqm();
	}
	virtual ~CommRec() {}
	
public:
	bool saveTodb(int &rowid);
	bool getXmDetl();
	bool getDevInfo();
	unsigned int getGroupID();
	unsigned int getNextGroupID();
	bool getSSInfo();
	//分组
	/*static*/ E_GRP_FLAG    m_seGrplast; // 0无 1 开始 2结束

protected:
	//from db
	unsigned int m_sbid;
	string 	     m_sbmc;
	unsigned int m_sblx;
	//string 	m_sblxmc;
	string	m_ip;
	string	m_fx;
	unsigned int m_ssid;
	string	m_ssmc;
	unsigned int /*string*/	m_ssfz;
	unsigned int/*string*/	m_sszl;

	string      m_ssbm;
	unsigned int  m_sslx;
	
	E_GRP_FLAG m_seGrp; // 0无 1 开始 2结束
	E_DATA_CLASS m_dtclass;

	unsigned int m_groupid;

	//xm from db
	unsigned int m_xmid;
	//string	m_xmbm;
	string	m_xmmc;
	
	//char        m_xysx;	
	//char		m_xylb;//$/@
	
	float 	m_sjz;
	string	m_sjzzf;	

	//jq from db
	float  m_sx;
	float  m_xx;
	unsigned short m_jqlb;


	MYSQL &m_dbconn;
	string      m_jqm;
	St_MsgRsp& m_stMsgRsp;
	E_PROTO_HDR m_hdr;
	E_PROTO_ADDR m_addr;
	E_PROTO_PROPER m_proper;
	string& m_def;
	unsigned short m_td;
	string m_sdata;
	string m_orgcmd;
	float m_data;
};

class AlmRec:public CommRec
{
public:
	AlmRec(MYSQL &dbconn,St_MsgRsp &stMsgRsp,E_PROTO_HDR hdr,E_PROTO_ADDR addr,E_PROTO_PROPER proper,string& def,unsigned short td,string& orgcmd,string sdata):CommRec(dbconn, stMsgRsp,hdr,addr,proper,def, td,orgcmd,sdata) {}
	virtual ~AlmRec() {}
public:
	//float  m_sjz_jq;
	E_ALM_LVL m_jqjb;
	string 		 m_jslr;
	bool saveTodb(int &rowid);
};
/*
class HBRec:public CommRec
{
protected:
	HBRec() {}
	virtual ~HBRec() {}
};


class CmdRec:public CommRec
{
protected:
	CmdRec() {}
	virtual ~CmdRec() {}
};

class DigitalRec:public CommRec
{
protected:
	DigitalRec() {}
	virtual ~DigitalRec() {}
};
*/

class OperationRec
{
public:
	OperationRec(MYSQL &dbconn,unsigned int dev_id,E_CMD_TYPE e_cmdtype,string& org_cmd):m_dbconn(dbconn),m_sbid(dev_id),m_xmbm(org_cmd),m_ecmdtype(e_cmdtype)
	{
	}
	OperationRec(MYSQL &dbconn,unsigned int dev_id,E_CMD_TYPE e_cmdtype,int  cmd_id):m_dbconn(dbconn),m_sbid(dev_id),m_xmid(cmd_id),m_ecmdtype(e_cmdtype)
	{
	}
	virtual ~OperationRec(){}
public:

	E_CMD_TYPE m_ecmdtype;
	
	unsigned int m_sbid;
	
	string	m_sbmc;
	string      m_jqm;
	unsigned short m_sblx;
	string	m_ip;
	string	m_fx;
	unsigned int	m_tdh;
	unsigned int	m_ssid;
	string	m_ssmc;
	unsigned int	m_ssfz;
	unsigned int	m_sszl;

	//string	m_ssfzmc;
	//string	m_sszlmc;

	unsigned int m_xmid;
	string      m_xmbm;
	string	m_xmmc;
	E_PROTO_HDR  m_xylb;//$/@

	MYSQL &m_dbconn;
public:
	bool saveTodb(int &rowid);
	bool getSSInfo();
	bool getDevInfo();
};



#endif //COMMREC_H
