#ifndef EM_H
#define EM_H

#include <string>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>   
//#include <string.h> 
#include <stdarg.h>


#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <error.h>
#include <sys/errno.h>
#include <sys/msg.h>
#include <errno.h>


#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <semaphore.h>
#include <getopt.h>
#include <mysql/mysql.h>

#include <set>
#include <vector>
#include <mysql/mysql.h>

#include "config.h"
#include "logger.h"
#include "chrt.h"
#include "signal.h"
#include "cfg.h"


using namespace std;



#define EM_VERSION	  "0.03"
//#define TCP_LSN_PORT 6660 //6660  6661
//#define UDP_SRV_PORT 5550
//#define UDP_CLNT_PORT 6440
#define MAX_TCP_SRV 10

/*消息队列键值*/   
#define MSG_KEY_REQ 0x12345  
#define MSG_KEY_RSP 0x54321


#define EM_CMD_LEN  12

/*
int devid 4bits
char type 1bit
char[] data 12bits
4+1+12 =17
*/
#define EM_REQ_LEN         17 
#define FENDMSG_LEN	 5

#define SQLBUFSIZE 		1024

//debug off
#define DBG_OFF 
//#define REQ_REPLY
//#define FALSE 0
//#define TRUE   1


#endif //EM_H