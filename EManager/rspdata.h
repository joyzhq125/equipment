#ifndef RSPDATA_H
#define RSPDATA_H
#include <mysql/mysql.h>
   
#define PRO_HDR_ADD        0
#define PRO_ADDR_ADD      1
#define PRO_PROPER_ADD   2
#define PRO_DEF_ADD        3
#define PRO_TD_ADD          7
#define PRO_DATA_ADD      8

typedef enum
{
	E_HDR_NONE = 0x00,
	E_HDR_NORMAL,
	E_HDR_REPLY,
	E_HDR_RESERVE
}E_PROTO_HDR;

typedef enum
{
	E_ADDR_NONE = 0x00,
	E_ADDR_NONEED ,
	E_ADDR_DEVCODE,
	E_ADDR_RESERVE1,
	E_ADDR_RESERVE2		
}E_PROTO_ADDR;	

typedef enum 
{
	E_PROTO_NONE = 0x00,
	E_PROTO_HB,
	E_PROTO_ALM,
	E_PROTO_CFG_ZS,	
	E_PROTO_CMD,
	E_PROTO_DIGIT_ZS, //Êý×Ö0x05
	E_PROTO_DIGIT_XS,
	E_PROTO_NEG_DIGIT_ZS,
	E_PROTO_NEG_DIGIT_XS,
	E_PROTO_CFG_XS,
	E_PROTO_NEG_CFG_ZS,
	E_PROTO_NEG_CFG_XS,
	E_PORTO_RESERVE1,
	E_PORTO_RESERVE2,
}E_PROTO_PROPER;
	

class RspData
{
public:
	RspData(St_MsgRsp &stMsgRsp,MYSQL &dbconn):m_stMsgRsp(stMsgRsp),m_dbconn(dbconn){}
	virtual ~RspData() {}

	bool saveReqReply(St_ReqReply *pstReqReply);
	unsigned int getDevId();
	bool sendReply();
	string getJqm();
	bool sendCmd(string& cmd);	
	bool sendCmdJqm(string& cmd);	
	St_MsgRsp& m_stMsgRsp;
	MYSQL& m_dbconn;
public:
	static RspData* preProc(St_MsgRsp &stMsgRsp,MYSQL &dbconn);
	static bool getProtoDtl(St_MsgRsp &stMsgRsp,E_PROTO_HDR &hdr,E_PROTO_ADDR &addr,E_PROTO_PROPER &proper,
		string& def,unsigned short& td,string& sdata,float &data);
	static E_PROTO_HDR m_hdr;
	static E_PROTO_ADDR m_addr;
	static E_PROTO_PROPER m_proper;
	static string m_def;
	static unsigned short m_td;
	static string m_sdata;
	static float   m_data;
	static string m_orgcmd;

	virtual bool doProc()=0;
private:	
	
};

class RspHB:public RspData
{
public:
	RspHB(St_MsgRsp &stMsgRsp,MYSQL &dbconn):RspData(stMsgRsp,dbconn){}
	virtual ~RspHB(){}
public:
	bool doProc();
};

class RspCmd:public RspData
{
public:
	RspCmd(St_MsgRsp &stMsgRsp,MYSQL &dbconn):RspData(stMsgRsp,dbconn){}
	virtual ~RspCmd(){}
public:
	bool doProc() {return true;}
};

class RspAlm:public RspData
{
public:
	RspAlm(St_MsgRsp &stMsgRsp,MYSQL &dbconn):RspData(stMsgRsp,dbconn){}
	virtual ~RspAlm(){}
public:
	bool doProc();
};

class RspCfg :public RspData
{
public:
	RspCfg(St_MsgRsp &stMsgRsp,MYSQL &dbconn):RspData(stMsgRsp,dbconn){}
	virtual ~RspCfg(){}
public:
	bool doProc();
};

class RspDigit:public RspData
{
public:
	RspDigit(St_MsgRsp &stMsgRsp,MYSQL &dbconn,E_PROTO_PROPER proper):RspData(stMsgRsp,dbconn),e_proper(proper){}
	virtual ~RspDigit(){}
	E_PROTO_PROPER e_proper;
public:
	bool doProc();
};

class RspReply:public RspData
{
public:
	RspReply(St_MsgRsp &stMsgRsp,MYSQL &dbconn):RspData(stMsgRsp,dbconn){}
	virtual ~RspReply(){}
public:
	bool doProc();
};
class RspDevCode:public RspData
{
public:
	RspDevCode(St_MsgRsp &stMsgRsp,MYSQL &dbconn):RspData(stMsgRsp,dbconn){}
	virtual ~RspDevCode(){}
public:
	bool doProc();
};

#endif //RSPDATA_H
