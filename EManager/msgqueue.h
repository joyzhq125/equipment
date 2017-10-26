
#ifndef MSGQUEUE_H
#define MSGQUEUE_H

class SockAddr;
class TcpInetSocket;
//#include "comms.h"
typedef struct tagSt_MsgReq
{
    SockAddr *sockAddr;
    char        recvData[EM_REQ_LEN];	
} St_MsgReq;
/*
class St_MsgReq
{
public:
    SockAddr *sockAddr;
    char        recvData[EM_REQ_LEN];	
};
*/
typedef struct tagSt_MsgRsp
{
    TcpInetSocket* tcpSrv;
    char        recvData[EM_CMD_LEN];
    unsigned short recvLen;
} St_MsgRsp;
/*
class St_MsgRsp
{
public:
    TcpInetServer* tcpSrv;
    char        recvData[EM_CMD_LEN];
};*/

enum {   
    MSG_TYPE_START = 0,    
    MSG_TYPE_MSG1,   
    MSG_TYPE_MSG2,   
    MSG_TYPE_MSG3,   
    MSG_TYPE_MSG4,   
    MSG_TYPE_MSG5,    
    MSG_TYPE_END   
};  

typedef struct tagSt_MsgReq_Queue
{  
	long msgtype;  
	St_MsgReq stMsgReq;  
 }St_MsgReq_Queue;  

typedef struct tagSt_MsgRsp_Queue
{  
	long msgtype;  
	St_MsgRsp stMsgRsp;  
 }St_MsgRsp_Queue;  


int SendMsgReq(St_MsgReq *pMsgReq);
int  GetMsgReq(St_MsgReq  *pstMsgReq);
int SendMsgRsp(St_MsgRsp *pMsgRsp);
int  GetMsgRsp(St_MsgRsp* pStMsgRsp);
void FreeMsgReqRsp();

#endif


