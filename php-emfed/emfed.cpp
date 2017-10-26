
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <error.h>
#include <sys/errno.h>
#include <sys/msg.h>
#include <errno.h>
#include <iostream>

#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <phpcpp.h>
#include "logger.h"
#include "emfed.h"

#include "comms.h"
#include "ecdr.h"


bool  EMFEnd::emfed_Inited = false;
//string EMFEnd::m_func;
//Php::Value EMFEnd::m_func;
void EMFEnd::emfed_Init(string func)
{
	EMFEnd* pFend = NULL;
	if(!EMFEnd::emfed_Inited)
	{
		pFend = new EMFEnd(func);
		pFend->start();
		//pFend->join();
		EMFEnd::emfed_Inited = true;
	}
	//Php::call("test1",11,12);
	//m_func = func;
}

//UDP_SRV_PORT_PHP
#define UDP_SRV_PORT_PHP 7770
int EMFEnd::run()
{
	UdpInetServer udpSrv;
	Select udpSelect;
	char revbuffer[EM_UNSOL_LEN];
	char tempbuffer[EM_UNSOL_LEN+1];
	Info("EMFEnd::run");
	if (!udpSrv.bind(UDP_SRV_PORT_PHP))
		return 0;
	if(!udpSelect.addReader(&udpSrv ))
		return 0;
	while(!mStop)  
	{  
		if ( mStop )
			break;
		if (udpSelect.wait() < 0)
			continue;
		Select::CommsList readable = udpSelect.getReadable();
		if ( readable.size() == 0 )
		{
			Warning( "Nothing to do" );
			continue;
		}  
	        for ( Select::CommsList::iterator iter = readable.begin(); iter != readable.end(); iter++)
	        {
	             if ( UdpInetSocket *socket = dynamic_cast<UdpInetSocket *>(*iter) )
	             {
	             	   int iRecv  = 0;	 
			   int readBytes = 0;
			   string rem_ip;
			   SockAddr * psockAddr = NULL;
			   memset(revbuffer,0,sizeof(revbuffer));
			   memset(tempbuffer,0,sizeof(tempbuffer));
			   while(readBytes < sizeof(revbuffer))
			   {
			   	/*
				if(psockAddr)
				{
					//rem_ip  = inet_ntoa(((sockaddr_in*)psockAddr->getAddr())->sin_addr);
					delete psockAddr;
				}
				*/
				//iRecv = socket->recvfrom(revbuffer+readBytes, sizeof(revbuffer) - readBytes,&psockAddr);
				iRecv = socket->recvfrom(revbuffer+readBytes, sizeof(revbuffer) - readBytes);
				if(iRecv <= 0)
				{
					//readBytes = iRecv;
					Error("rev data length is 0");
					break;
				}
				readBytes += iRecv;
	             	   }	
			   if(psockAddr)
			   {
				   rem_ip  = inet_ntoa(((sockaddr_in*)psockAddr->getAddr())->sin_addr);	
				   //delete psockAddr; //free later
				   Info("remote ip is %s",rem_ip.c_str());
				   psockAddr = NULL; 
			   }
			   /*
			   if(rem_ip.empty())
			   {
			   	Error("Got %d bytes on sd %d remip is empty", readBytes, socket->getReadDesc());
				continue;
			   }
			   else
	                 {
	                 	Info("Got %d bytes on sd %d remip %s", readBytes, socket->getReadDesc(),rem_ip.c_str());
			   }*/
	                 if ( readBytes > 0 )
	                 {
	                      //to do something
	                      //psockAddr + revbuffer
	                 	strncpy(tempbuffer, revbuffer, readBytes);
				Info("EMFEnd::run revbuffer is %s",tempbuffer);	
				
				St_ecdrDecoder *pstDecoder = NULL;
				int type = 0;
				int sid = 0;
				pstDecoder = ecdr_CreateDecoder((unsigned char *)tempbuffer,EM_UNSOL_LEN);
				ecdr_DecodeUint8(pstDecoder, (unsigned char*)&type);
				ecdr_DecodeUint32(pstDecoder, (unsigned int*)&sid);
				ecdr_DestroyDecoder(pstDecoder);		
				Info("EMFEnd::type=%u,sid=%d",type,sid);

				
				if (!m_func.isCallable()) 
				{
					Error("m_func not a function");
					//throw Php::Exception("not a function");
				}
				else
				{
					m_func();
				}
				/*
				if(!m_func.empty())
				{
					Info("EMFEnd::func=%s",m_func.c_str());
				 	Php::Value function(m_func);
					if (!function.isCallable()) 
					{
						Error("m_func not a function");
					}
					else
					{
						Php::call(m_func.c_str(),11,12);
					}
				}*/

	                 }
	                 else
	                 {
				if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
				{
				}
				else
				{
					//udpSelect.deleteReader(socket);
					//delete socket;
				}
	                 }
	             }
	             else
	             {
	                 Warning( "Barfed" );
	             }
	         }		
		
	}
	Info("EMFEnd::term");
	udpSrv.close();
	return 0;
}