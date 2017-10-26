
/*
emanager  daemon
*/

#include "em.h"
#include "comms.h"
#include "ecdr.h"
#include "cfg.h"
#include "signal.h"
#include "msghdl.h"
#include "msgqueue.h"
#include "inet.h"
#include "misc.h"
#include "cfg.h"
#include "db.h"
#include "emanager.h"

Inet **inet = NULL;


void Usage()
{
	fprintf( stderr, "emanager or emanager -v\n" );

	fprintf( stderr, "Options:\n" );
	fprintf( stderr, "  -v --version : get version\n" );
	fprintf( stderr, "  -h, --help  :help\n" );
	exit( 0 );
}

int main( int argc, char *argv[] )
{
	/*
	static struct option long_options[] = {
			{"version", 0, 0, 'v'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0}
		};	
	srand( getpid() * time(0));
	while (1)
	{
		int option_index = 0;
		int c = getopt_long (argc, argv, "v:h", long_options, &option_index);
		if (c == -1)
		{
			break;
		}
		switch (c)
		{
			case 'v':
				file = optarg;
				break;	
			case 'h':
			case '?':
				Usage();
				break;
			default:
				//fprintf( stderr, "?? getopt returned character code 0%o ??\n", c );
				break;
		}
	}

	if (optind < argc)
	{
		fprintf( stderr, "Extraneous options, " );
		while (optind < argc)
			printf ("%s ", argv[optind++]);
		printf ("\n");
		Usage();
	}
	*/
	logInit( "EM" );
	LoadConfig();	
	//dbConnect();
	//ssedetect();
	SetDefaultTermHandler();
	SetDefaultDieHandler();
	SetDefaultHupHandler();
	FEnd fend;
	inet = new Inet*[emConfig.TCP_SRV_NUM];
	MsgReqThread msgReq;
	MsgRspThread msgRsp;
	msgReq.start();
	msgRsp.start();
	fend.start();
	int idx = 0;
	for(idx = 0;idx < emConfig.TCP_SRV_NUM;idx++)
	{		
		inet[idx] = new Inet;
		inet[idx]->start();
	}
	Misc tm_misc(emConfig.MISC_INTERVAL,true);
	//CheckMethod tm_chk(emConfig.CHK_INTERVAL,true); 
	msgReq.join();
	msgRsp.join();
	fend.join();
	for( idx = 0;idx < emConfig.TCP_SRV_NUM;idx++)
	{		
		inet[idx]->join();	
	}
	logTerm();
	FreeMsgReqRsp();
	for(idx = 0;idx < emConfig.TCP_SRV_NUM;idx++)
	{		
		delete inet[idx];
	}
}


int FEnd::run()
{
	UdpInetServer udpSrv;
	Select udpSelect;
	char revbuffer[EM_REQ_LEN];
	char tempbuffer[EM_REQ_LEN+1];
	Info("FEnd::run");
	if (!udpSrv.bind(emConfig.UDP_SRV_PORT))
		return 0;
	if(!udpSelect.addReader(&udpSrv ))
		return 0;
	while(!mStop && !g_terminate)  
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
			   St_MsgReq stMsgReq = {0,};
			   memset(revbuffer,0,sizeof(revbuffer));
			   memset(tempbuffer,0,sizeof(tempbuffer));
			   while(readBytes < sizeof(revbuffer))
			   {
				if(psockAddr)
				{
					//rem_ip  = inet_ntoa(((sockaddr_in*)psockAddr->getAddr())->sin_addr);
					delete psockAddr;
				}
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
				   Info("remote ip is %s",rem_ip.empty()?"":rem_ip.c_str());
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
			   }
			   */
	                 if ( readBytes > 0 )
	                 {
	                      //to do something
	                      //psockAddr + revbuffer
		                 	strncpy(tempbuffer, revbuffer, readBytes);
					//Info("FEnd::run revbuffer is %s",tempbuffer);	
					memset(&stMsgReq,0,sizeof(St_MsgReq));
					stMsgReq.sockAddr = psockAddr;
					memcpy(stMsgReq.recvData,revbuffer,sizeof(revbuffer));
	                      	if (0 == SendMsgReq(&stMsgReq))
                      		{
                      			Error("sendmsgreq fail");
                      		}
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
	Info("FEnd::term");
	udpSrv.close();
	return 0;
}


int unsFEndMsg(St_UnsolFEndMsg& msg)
{
	Info("unsFEndMsg");
	UdpInetClient udpClnt;
	int ilen = 0;
	//if (!udpClnt.bind(UDP_CLNT_PORT))
	//	return 0;
	//udpClnt.connect(const char * host, int port);
	//encode
	SockAddrInet sockAddr;
	St_ecdrEncoder * pstEncoder = NULL;
	pstEncoder = ecdr_CreateEncoder(FENDMSG_LEN);
	ecdr_EncodeUint8(pstEncoder, (unsigned char*)&msg.msgtype);
	ecdr_EncodeUint32(pstEncoder,(unsigned int* )&msg.jlid);
	if(FENDMSG_LEN  != pstEncoder->iPosition)
	{
		Error("RspHB::doProc encode fail");
		ecdr_DestroyEncoder( pstEncoder );
		return 0;
	}
	//sockAddr.resolve(serv, port,"udp");
	if((ilen = udpClnt.sendto(pstEncoder->pucData,pstEncoder->iLength,&sockAddr)) < pstEncoder->iLength)
	{
		Error("unsFEndMsg sendto fail,send length %d,org length %d",ilen, pstEncoder->iLength);
	}
	ecdr_DestroyEncoder( pstEncoder );
	return 1;
}

int rspFEndMsg(St_RspFEndMsg & msg)
{
	Info("rspFEndMsg");
	UdpInetClient udpClnt;
	int ilen = 0;
	//if (!udpClnt.bind(UDP_CLNT_PORT))
	//	return 0;
	//udpClnt.connect(const char * host, int port);
	//encode
	SockAddrInet sockAddr;
	St_ecdrEncoder * pstEncoder = NULL;
	pstEncoder = ecdr_CreateEncoder(FENDMSG_LEN);
	ecdr_EncodeUint8(pstEncoder, (unsigned char*)&msg.restype);
	ecdr_EncodeUint32(pstEncoder,(unsigned int* )&msg.txsjid);
	if(FENDMSG_LEN  != pstEncoder->iPosition)
	{
		Error("RspHB::doProc encode fail");
		ecdr_DestroyEncoder( pstEncoder );
		return 0;
	}
	//sockAddr.resolve(serv, port,"udp");
	if((ilen = udpClnt.sendto(pstEncoder->pucData,pstEncoder->iLength,&sockAddr)) < pstEncoder->iLength)
	{
		Error("rspFEndMsg sendto fail,send length %d,org length %d",ilen, pstEncoder->iLength);
	}
	ecdr_DestroyEncoder( pstEncoder );
	return 1;
}