#ifndef INET_H
#define INET_H

#include "thread.h"



class Inet : public Thread
{

public:
    Inet():mStop(false) {}
    virtual ~Inet() {}
	
    void stop()
    {
        mStop = true;
    }
    bool stopped() const
    {
        return( mStop );
    }
    int run();
    //static void delUnuseSockAll(string& jqm);
    static void  delUnuseSockAll(string& jqm,string ip);
    static TcpInetSocket * findConnSockAll(string& jqm,string ip="");	
    static void closePendingSock();
    static bool sendCmd(TcpInetSocket* socket, const string& cmd);
private:
    bool mStop;
    TcpInetServer m_tcpServer;
    Select m_tcpSelect;
    bool findUnuseSock(string& jqm);
    TcpInetSocket * findConnSock(string& jqm,string ip="");	
    void findPendCloseSock();

};



#endif //INET_H
