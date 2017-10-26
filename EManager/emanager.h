#ifndef EMANAGER_H
#define EMANAGER_H

#include "thread.h"
//typedef void *(* THREAD)(void *);

//
typedef enum tagE_UnsolFEndMsgType
{
      //public
	E_ALARM = 0x01, //警报
	E_NCDEV,            //新控制主机
	E_IPCHG,       //ip 地址改变
	E_NCMD,             //新的上报命令
	E_OFFLINE,	  //主机离线
	E_CMDCHG,    //命令属性更改
	
	//option
	E_NEWDATA  //新数据
	
}E_UnsolFEndMsgType;

typedef struct tagSt_UnsolFEndMsg
{
	E_UnsolFEndMsgType msgtype;
	unsigned int    jlid;	
}St_UnsolFEndMsg;

//
typedef enum tagE_RspFEndResType
{
	E_SUC = 0x01, //成功
	E_FAIL,            //失败
	E_TIMEOUT       //超时
	
}E_RspFEndResType;

typedef struct tagSt_RspFEndMsg
{
	E_RspFEndResType restype;
	unsigned int    txsjid;	
}St_RspFEndMsg;





class FEnd: public Thread
{
public:
    FEnd():mStop(false) {}
    virtual ~FEnd() {}
	
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




//err code  
#define SECCESS                                   0x00
#define GENERAL_ERROR                      0x01                 
#define NOT_SUPPORT                          0x02          
#define INVALID_PARAMETER              0x03      
#define INVALID_FORMAT			     0x04

int unsFEndMsg(St_UnsolFEndMsg& msg);
int rspFEndMsg(St_RspFEndMsg & msg);

#endif //WIFUNC_H
