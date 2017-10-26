#ifndef REQDATA_H
#define REQDATA_H
#include <mysql/mysql.h>
typedef enum
{
	CMD_NONE = 0x00,
	CMD_ORG,
	//CMD_ID,
	CMD_DIGIT,
	CMD_STR,
	//CMD_DIGIT_XS,
	CMD_NORMAL		
}E_CMD_TYPE;

#define REQ_DEVID_ADDR    0
#define REQ_TYPEID_ADDR  4
#define REQ_CMD_ADDR       5

class ReqData
{
public:
	ReqData(St_MsgReq &stMsgReq,MYSQL &dbconn) :m_stMsgReq(stMsgReq),m_dbconn(dbconn){}
	virtual ~ReqData(){}

private:

public:	
	MYSQL &m_dbconn;
	St_MsgReq& m_stMsgReq;
	static ReqData* preProc(St_MsgReq &stMsgReq,MYSQL &dbconn);
	virtual bool doProc()=0;
	bool getJqm(unsigned devid,string& jqm,string& ip);
};

class ReqCmd:public ReqData
{
public:
	ReqCmd(St_MsgReq &stMsgReq,MYSQL &dbconn):ReqData(stMsgReq,dbconn){}
	virtual ~ReqCmd(){}
public:
	bool doProc(){return true;}
};
class ReqCmdOrg:public ReqData
{
public:
	ReqCmdOrg(St_MsgReq &stMsgReq,MYSQL &dbconn):ReqData(stMsgReq,dbconn){}
	virtual ~ReqCmdOrg(){}
public:
	bool doProc();
};
class ReqCmdDigit:public ReqData
{
public:
	ReqCmdDigit(St_MsgReq &stMsgReq,MYSQL &dbconn,E_CMD_TYPE cmdtype):ReqData(stMsgReq,dbconn),e_cmdtype(cmdtype){}
	virtual ~ReqCmdDigit(){}
	E_CMD_TYPE e_cmdtype;
public:
	bool doProc(){return true;};
};

class ReqCmdId:public ReqData
{
public:
	ReqCmdId(St_MsgReq &stMsgReq,MYSQL &dbconn):ReqData(stMsgReq,dbconn){}
	virtual ~ReqCmdId(){}
public:
	bool doProc();
};

#endif //REQDATA_H