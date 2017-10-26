
#include "em.h"
#include "msgqueue.h"

static int iMsg_Req = 0;
static int iMsg_Rsp = 0;
static bool req_init = false;
static bool rsp_init = false;

static Mutex Mutex_Req;
static Mutex Mutex_Rsp;

int Msg_Init( int msgKey );
int Msg_Kill(int qid);

//请求
int SendMsgReq(St_MsgReq *pMsgReq)
{
	//int iMsg_id = 0;
	int ret_value = 0;  
	St_MsgReq_Queue stMsgReq_Queue = {0};
	
	Info("SendMsgReq");
	iMsg_Req = Msg_Init(MSG_KEY_REQ);
	if(!req_init)
	{
		Msg_Kill(iMsg_Req);
		iMsg_Req = Msg_Init(MSG_KEY_REQ);
		req_init = true;	
	}
	
	memset(&stMsgReq_Queue,0,sizeof(St_MsgReq_Queue));
	stMsgReq_Queue.msgtype = MSG_TYPE_MSG1;
	memcpy(&stMsgReq_Queue.stMsgReq, pMsgReq,sizeof(St_MsgReq));	
	Mutex_Req.lock(2);
	ret_value = msgsnd(iMsg_Req, &stMsgReq_Queue, sizeof(St_MsgReq_Queue), IPC_NOWAIT);
	Mutex_Req.unlock();
	if ( ret_value ==  -1 ) 
	//if (ret_value != 0)
	{  
	       Error("msgsnd() write msg failed,errno=%d[%s]\n",errno,strerror(errno));  
		return 0;
	} 

	return 1;
	
}


int  GetMsgReq(St_MsgReq  *pstMsgReq)	
{
	int ret_value = 0;
	St_MsgReq_Queue stMsgReq_Queue = {0};

	if (NULL == pstMsgReq)
	{
		return 0;
	}
	Info("GetMsgReq");
	iMsg_Req = Msg_Init(MSG_KEY_REQ);
	if(!req_init)
	{
		Msg_Kill(iMsg_Req);
		iMsg_Req = Msg_Init(MSG_KEY_REQ);
		req_init = true;	
	}	
	ret_value = msgrcv(iMsg_Req,&stMsgReq_Queue,sizeof(St_MsgReq_Queue),MSG_TYPE_MSG1,0); 
	if (ret_value == -1)
	//if (ret_value != 0)
	{
		Error("GetMsgReq:msgrcv() get msg failed,errno=%d[%s]\n",errno,strerror(errno));  
		return 0;
	}
	else
	{
		pstMsgReq->sockAddr = stMsgReq_Queue.stMsgReq.sockAddr;
		memcpy(pstMsgReq->recvData,stMsgReq_Queue.stMsgReq.recvData,sizeof(pstMsgReq->recvData));
	}	
	return 1;
	
}


int SendMsgRsp(St_MsgRsp *pMsgRsp)
{
	int ret_value = 0;  
	St_MsgRsp_Queue stMsgRsp_Queue = {0};

	//memset(pSt_MsgRsp,0,,sizeof(St_MsgRsp));
	//memcpy(pSt_MsgRsp,pMsgRsp,sizeof(St_MsgRsp));
	//add queue here
	Info("SendMsgRsp");
	iMsg_Rsp = Msg_Init(MSG_KEY_RSP);	
	if(!rsp_init)
	{
		Msg_Kill(iMsg_Rsp);
		iMsg_Rsp = Msg_Init(MSG_KEY_RSP);
		rsp_init = true;	
	}
	memset(&stMsgRsp_Queue,0,sizeof(St_MsgRsp_Queue));
	stMsgRsp_Queue.msgtype = MSG_TYPE_MSG2;
	memcpy(&stMsgRsp_Queue.stMsgRsp, pMsgRsp,sizeof(St_MsgRsp));
	Mutex_Rsp.lock(2);
	ret_value = msgsnd(iMsg_Rsp, &stMsgRsp_Queue, sizeof(St_MsgRsp_Queue), IPC_NOWAIT);
	Mutex_Rsp.unlock();
	if ( ret_value ==  -1 ) 
	//if (ret_value != 0)
	{  
	       Error("SendMsgRsp:msgsnd() write msg failed,errno=%d[%s]",errno,strerror(errno)); 
		return 0;
	} 
	
	return 1;
}

int  GetMsgRsp(St_MsgRsp* pStMsgRsp)
{
	int ret_value = 0;
	St_MsgRsp_Queue stMsgRsp_Queue = {0};
	if(NULL == pStMsgRsp)
	{
		Info("GetMsgRsp null pointer");
		return 0;
	}
	Info("GetMsgRsp");
	iMsg_Rsp = Msg_Init(MSG_KEY_RSP);
	if(!rsp_init)
	{
		Msg_Kill(iMsg_Rsp);
		iMsg_Rsp = Msg_Init(MSG_KEY_RSP);
		rsp_init = true;	
	}
	ret_value = msgrcv(iMsg_Rsp,&stMsgRsp_Queue,sizeof(St_MsgRsp_Queue),MSG_TYPE_MSG2,0); 
	if (ret_value == -1)
	//if (ret_value != 0)
	{
		Error("GetMsgRsp:msgrcv() get msg failed,errno=%d[%s]",errno,strerror(errno));  
		return 0;
	}
	else
	{
		pStMsgRsp->tcpSrv = stMsgRsp_Queue.stMsgRsp.tcpSrv;
		pStMsgRsp->recvLen = stMsgRsp_Queue.stMsgRsp.recvLen;
		memcpy(pStMsgRsp->recvData,stMsgRsp_Queue.stMsgRsp.recvData,sizeof(pStMsgRsp->recvData));
	}
	return 1;

}


void FreeMsgReqRsp()
{
	if(iMsg_Req > 0)
	{
		Msg_Kill(iMsg_Req);
	}
	if(iMsg_Rsp > 0)
	{
		Msg_Kill(iMsg_Rsp);
	}	
}

/*  
消息队列初始化  
msgKey:消息队列键值  
qid:返回值，消息队列id  
*/   
int Msg_Init( int msgKey )   
{   
	int qid;   
	key_t key = msgKey;   
	qid = msgget(key,0);   
	if(qid < 0)
	{   
		/*  
		打开不成功，表明未被创建  
		现在可以按照标准方式创建消息队列  
		*/   
		qid = msgget(key,IPC_CREAT|0666);   

		if(qid <0)
		{  
			Error("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));  
			return 0;
		}  

		Info("Create msg queue id:%d",qid);
	}   
	Info("msg queue id:%d",qid);
	return qid;   
}   



/*  
杀死消息队列  
qid:消息队列id  
*/   
int Msg_Kill(int qid)   
{   
	msgctl(qid, IPC_RMID, NULL);    
	Info("Kill queue id:%d",qid);
	return 1;   
}  





