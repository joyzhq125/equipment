

#include "em.h"
#include "comms.h"
#include "msghdl.h"
#include "msgqueue.h"
#include "inet.h"
#include "db.h"

#define RETRY_TIMES               10
#define PENDCLOSE_INTERVAL 1200  // 20 minutes 10*60

extern Inet **inet;

int Inet::run()
{
	Info("Inet::run");
	//TcpInetServer tcpServer;
	//Select tcpSelect;
	char revbuffer[EM_CMD_LEN] = {0,};
	char tempbuffer[EM_CMD_LEN+1] = {0,};
	static unsigned short tcp_svr_port = emConfig.TCP_SRV_PORT;	
	if (!m_tcpServer.bind(tcp_svr_port ++))
		return 0;	
	//proc/sys/net/core/somaxconn	default:128
	if(!m_tcpServer.listen())
		return 0;
	//m_tcpServer.setBlocking(false);
	m_tcpSelect.setLsnFd(&m_tcpServer);
	if(!m_tcpSelect.addReaderLsn(&m_tcpServer ))
		return 0;
	while(!mStop && !g_terminate)  
	{  
		 if ( mStop )
			break;
		if (m_tcpSelect.waitLsn() <= 0)
			continue;
		Select::CommsList readable = m_tcpSelect.getReadable();
		if ( readable.size() == 0 )
		{
			Warning( "InetThread::run Nothing to do" );
			continue;
		}  
	        for ( Select::CommsList::iterator iter = readable.begin(); iter != readable.end(); iter++)
	        {	
	             if (TcpInetSocket *socket = dynamic_cast<TcpInetSocket *>(*iter) )
	             {
 			   int iRecv = 0;
			   int readBytes = 0;
			   string remote_ip = socket->getDevSbip();
			   string jqm = socket->getDevSbJqm();	
			   St_MsgRsp stMsgRsp = {0,};
			   
	             	   memset(revbuffer,0,sizeof(revbuffer));
			   memset(tempbuffer,0,sizeof(tempbuffer));
			   #if 1			   
			   iRecv = socket->recv(revbuffer, sizeof(revbuffer));
			   readBytes = iRecv;
			   #else
			   while(readBytes < sizeof(revbuffer))
			   {
				iRecv = socket->recv(revbuffer+readBytes, sizeof(revbuffer) - readBytes);
				if(iRecv <= 0)
				{
					readBytes = iRecv;
					Error("rev data length is %d",iRecv);
					break;
				}
				readBytes += iRecv;
	             	   }
			   #endif

	                 Info("Got %d bytes on sd %d", readBytes, socket->getReadDesc() );
	                 if (readBytes >0 )
	                 {  
	                 	memcpy(tempbuffer, revbuffer, readBytes);
				//Info("Inet::run revbuffer is %s",tempbuffer);
				if (!jqm.empty() && !remote_ip.empty())
				{
					Info("recv from  ip: %s,jqm: %s,revbuffer is %s",remote_ip.c_str(),jqm.c_str(),tempbuffer);
				}
				else if( !remote_ip.empty())
				{
					Info("recv from  ip: %s,revbuffer is %s",remote_ip.c_str(),tempbuffer);
				}
				else 
				{
					Error("recv ip is empty");
					continue;
				}
				
				memset(&stMsgRsp,0,sizeof(St_MsgRsp));
				stMsgRsp.tcpSrv = socket;
				stMsgRsp.recvLen = readBytes;
				memcpy(stMsgRsp.recvData,tempbuffer,readBytes);
				if(/*readBytes != socket->pipe_write(tempbuffer,readBytes) || */0 == SendMsgRsp(&stMsgRsp))
				{
					Error("Inet::run sendmsgrsp fail");
				}
	                     
	                 }
	                 else
	                 {
	                 	Error("errno = %d, error = %s", errno, strerror(errno));
				if((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) && socket->m_retrytimes < RETRY_TIMES)
				{
					Error("errno = %d, error = %s,continue", errno, strerror(errno));
					socket->m_retrytimes ++;
					continue;
				}
				else
				{
					if (!jqm.empty() && !remote_ip.empty())
					{
						Info("release sd:%d,ip:%s,sbbm:%s",socket->getReadDesc(),remote_ip.c_str(),jqm.c_str());
					}
					else if( !remote_ip.empty())
					{
						Info("release sd:%d,ip:%s",socket->getReadDesc(),remote_ip.c_str());
					}
					else
					{
						Info("release sd:%d",socket->getReadDesc());
					}
					m_tcpSelect.deleteReaderLsn(socket);
					socket->close();
					/*
					if(!socket->check_pending_close())
					{
						delete socket;
					}
					
					else
					*/
					{
						m_tcpSelect.addPendClose(socket);
						socket->setCloseTime();
						Info("Inet::run pending close sd 0x%x",socket->getReadDesc());
					}
					
				}
	                 }
	             }
	             else
	             {
	                 Warning( "Barfed" );
	             }
	         }		
		
	}
	Info(" Inet::term");
	m_tcpServer.close();
	return 0;
}

TcpInetSocket * Inet::findConnSock(string& jqm,string ip)
{
	Select::CommsSet readers = m_tcpSelect.getReaders();
	if(!ip.empty())
	{
		for (Select::CommsSet::iterator iter = readers.begin(); iter != readers.end(); iter++)
		{
			if (ip == (*iter)->getDevSbip() && (*iter)->isOpen())
			{	
				return dynamic_cast<TcpInetSocket *>(*iter);
			}
		}
	}
	else if(!jqm.empty())
	{
		for (Select::CommsSet::iterator iter = readers.begin(); iter != readers.end(); iter++)
		{
			if (jqm == (*iter)->getDevSbJqm() && (*iter)->isOpen())
			{	
				return dynamic_cast<TcpInetSocket *>(*iter);
			}
		}
	}
	return NULL;
}


bool Inet::findUnuseSock(string& jqm)
{
	if(!jqm.empty())
	{
		Debug(1,"Inet::findUnuseSock jqm is %s",jqm.c_str());
		Select::CommsSet readers = m_tcpSelect.getReaders();
		for (Select::CommsSet::iterator iter = readers.begin(); iter != readers.end(); iter++)
		{
			if (jqm == (*iter)->getDevSbJqm() && (*iter)->isOpen())
			{	
				Info("Inet::findUnuseSock %s found and delete",jqm.c_str());
				TcpInetSocket* socket = dynamic_cast<TcpInetSocket *>(*iter);
				m_tcpSelect.deleteReaderLsn(socket);
				socket->close();
				delete socket;
				return true;
			}
		}
	}
	return false;
}

void  Inet::findPendCloseSock()
{
	Debug(1,"pendclose size is %d s",m_tcpSelect.getPendClose().size());
	if (m_tcpSelect.getPendClose().size() > 0 )
	{		  
	        for ( Select::CommsList::iterator iter = m_tcpSelect.getPendClose().begin(); iter != m_tcpSelect.getPendClose().end();/* iter--*/)
	        {
	             if(TcpInetSocket *socket = dynamic_cast<TcpInetSocket *>(*iter))
	             {
				double diff = tvDiffSec(socket->m_closetime);
				Debug(1,"found pending close sd,diff is %f",diff);
				if(diff > PENDCLOSE_INTERVAL)
				{
					Info("Inet::findPendCloseSock close pending close sd 0x%x ",socket);
					iter = m_tcpSelect.deletePendClose(iter);
					delete socket;
				}
				else
				{
					iter++;
				}
	             }
	        }
	}
	Debug(1,"pendclose size is %d e",m_tcpSelect.getPendClose().size());
}


bool Inet::sendCmd(TcpInetSocket* socket, const string& cmd)
{	
	unsigned int sendnum = 0;
	Info("Inet::sendCmd %s",cmd.empty()?"":cmd.c_str());
	if(socket && socket->isOpen() && !cmd.empty() && EM_CMD_LEN == socket->send_nonblock(cmd))
	{
		return true;
	}
	else
	{
		return false;
	}
}

TcpInetSocket *  Inet::findConnSockAll(string& jqm,string ip)
{
	TcpInetSocket * pSocket = NULL;
	if(!jqm.empty())
	{
		for(int idx = 0;idx < emConfig.TCP_SRV_NUM; idx++)
		{
			Inet * net =  inet[idx];
			if(net && (pSocket = net->findConnSock(jqm)) != NULL)
			{
				Info("findConnSockAll1 sd=%d",pSocket->getReadDesc());
				return pSocket;
			}
		}
	}
	if(!ip.empty())
	{
		for(int idx = 0;idx < emConfig.TCP_SRV_NUM; idx++)
		{
			Inet * net =  inet[idx];
			if(net && (pSocket = net->findConnSock(jqm,ip)) != NULL)
			{
				Info("findConnSockAll2 sd=%d",pSocket->getReadDesc());
				return pSocket;
			}
		}
	}
	Error("cannot find socket");
	return NULL;
}


void Inet::closePendingSock()
{
	for(int idx = 0;idx < emConfig.TCP_SRV_NUM; idx++)
	{
		Inet * net =  inet[idx];
		if(net )
		{
			net->findPendCloseSock();
		}
	}
}

void  Inet::delUnuseSockAll(string& jqm,string ip)
{
	TcpInetSocket * pSocket = NULL;
	if(!jqm.empty())
	{
		for(int idx = 0;idx < emConfig.TCP_SRV_NUM; idx++)
		{
			Inet * net =  inet[idx];
			if(net && (pSocket = net->findConnSock(jqm)) != NULL)
			{
				if(pSocket)
				{
					Info("delUnuseSockAll jqm found sd =%d,%s,%s",pSocket->getReadDesc(),jqm.empty()?"":jqm.c_str(),ip.empty()?"":ip.c_str());
					net->m_tcpSelect.deleteReaderLsn(pSocket);
					pSocket->close();		
					
					net->m_tcpSelect.addPendClose(pSocket);
					pSocket->setCloseTime();
					Info("delUnuseSockAll jqm pending close sd 0x%x",pSocket->getReadDesc());
					//delete pSocket;
					return;
				}
			}
		}
	}
	if(!ip.empty())
	{
		for(int idx = 0;idx < emConfig.TCP_SRV_NUM; idx++)
		{
			Inet * net =  inet[idx];
			if(net && (pSocket = net->findConnSock(jqm,ip)) != NULL)
			{
				if(pSocket)
				{
					Info("delUnuseSockAll ip found sd =%d,%s,%s",pSocket->getReadDesc(),jqm.empty()?"":jqm.c_str(),ip.empty()?"":ip.c_str());
					net->m_tcpSelect.deleteReaderLsn(pSocket);
					pSocket->close();
					
					net->m_tcpSelect.addPendClose(pSocket);
					pSocket->setCloseTime();
					Info("delUnuseSockAll ip pending close sd 0x%x",pSocket->getReadDesc());					
					//delete pSocket;
					return;
				}
			}
		}
	}
	Warning("delUnuseSockAll not found");
	
}

/*
bool sendCmd(const string& host,int port,const string& cmd)
{
	TcpInetClient tcpClnt;
	int sendLen = 0;
	if (cmd.empty())
	{
		Error("cmd is empty");
		return false;
	}
	Info("send cmd %s,len %d",cmd.c_str(),cmd.length());
	if (!tcpClnt.connect( host.c_str(), port))
	{
		Error("connect fail");
		return false;
	}
	if((sendLen = tcpClnt.send(cmd.c_str(), cmd.length()))< cmd.length())
	{
		Error("send fail len is %d",sendLen);
	}
	Info("send cmd ok");
	return true;
}
*/
