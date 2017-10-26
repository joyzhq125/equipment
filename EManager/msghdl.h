#ifndef MSGHDL_H
#define MSGHDL_H
#include <set>
#include <vector>
#include "msgqueue.h"
#include "thread.h"
typedef struct
{
	unsigned int devid;
	St_MsgReq stReq;
	timeval   sndTime;
	timeval   rspTime;
}St_ReqReply;

typedef std::vector<St_ReqReply *>  ReqList;

class MsgThread:public Thread
{
public:
	MsgThread() {dbconn();}
	virtual ~MsgThread() {}

	MYSQL m_DbConnection;
protected:
	bool dbconn();
};

class MsgReqThread : public MsgThread
{
public:
	MsgReqThread():mStop(false) {}
	virtual ~MsgReqThread() {}
	void stop()
	{
		mStop = true;
	}
	bool stopped() const
	{
		return( mStop );
	}
	int run();
	static ReqList m_reqCmdList;
private:
	bool mStop;
	
};


class MsgRspThread : public MsgThread
{
public:
	MsgRspThread():mStop(false) {}
	virtual ~MsgRspThread() {}
	void stop()
	{
		mStop = true;
	}
	bool stopped() const
	{
		return( mStop );
	}
	int run();
private:
	bool mStop;	
};


#endif //MSGHDL_H