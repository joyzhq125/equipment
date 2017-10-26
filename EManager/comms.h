

#ifndef COMMS_H
#define COMMS_H

#include "exception.h"
#include "pipe.h"
#include "time.h"
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <sys/un.h>

#include <set>
#include <vector>

#include <sys/epoll.h>
#include <sys/resource.h>

#define EPOOL

class Epoll;

typedef enum
{
	E_GRP_NONE = 0x00,
	E_GRP_ST, //���鿪ʼ
	E_GRP_MID,  //��������
	E_GRP_ED //�������
}E_GRP_FLAG;

class CommsException : public Exception
{
public:
    CommsException( const std::string &message ) : Exception( message )
    {
    }
};

class CommsBase
{
protected:
	const int	&mRd;
	const int	&mWd;
	


	std::string  m_remote_ip;
	E_GRP_FLAG    m_seGrplast;
	PIPE 		m_pipe;	
	//bool 		m_bpendclose;
public:
	//bool 		m_bpendclose;
	timeval 		m_closetime;
	int 			m_retrytimes;
	int 			m_devCodetimes;
	std::string  	m_sbjqm;
protected:
	CommsBase( int &rd, int &wd ) : mRd( rd ), mWd( wd ),m_seGrplast(E_GRP_NONE),m_retrytimes(0),m_devCodetimes(0)
	{
	}
	virtual ~CommsBase()
	{
	}

public:
	virtual bool close()=0;
	virtual bool isOpen() const=0;
	virtual bool isClosed() const=0;
	virtual bool setBlocking( bool blocking )=0;

public:
	int getReadDesc() const
    {
        return( mRd );
    }
	int getWriteDesc() const
    {
        return( mWd );
    }
	int getMaxDesc() const
    {
        return( mRd>mWd?mRd:mWd );
    }
	virtual int read( void *msg, int len )
	{
		ssize_t nBytes = ::read( mRd, msg, len );
        if ( nBytes < 0 )
            Debug( 1, "Read of %d bytes max on rd %d failed: %s", len, mRd, strerror(errno) );
		return( nBytes );
	}
	virtual int write( const void *msg, int len )
	{
		ssize_t nBytes = ::write( mWd, msg, len );
        if ( nBytes < 0 )
            Debug( 1, "Write of %d bytes on wd %d failed: %s", len, mWd, strerror(errno) );
		return( nBytes );
	}
	virtual int readV( const struct iovec *iov, int iovcnt )
	{
		int nBytes = ::readv( mRd, iov, iovcnt );
        if ( nBytes < 0 )
            Debug( 1, "Readv of %d buffers max on rd %d failed: %s", iovcnt, mRd, strerror(errno) );
		return( nBytes );
	}
	virtual int writeV( const struct iovec *iov, int iovcnt )
	{
		ssize_t nBytes = ::writev( mWd, iov, iovcnt );
        if ( nBytes < 0 )
            Debug( 1, "Writev of %d buffers on wd %d failed: %s", iovcnt, mWd, strerror(errno) );
		return( nBytes );
	}
	virtual int readV( int iovcnt, /* const void *msg1, int len1, */ ... );
	virtual int writeV( int iovcnt, /* const void *msg1, int len1, */ ... );
	//��������
       void setDevSbJqm(std::string& sbbm) 
       {
      		 m_sbjqm = sbbm;
       }
	//
	void  setDevSbip(std::string& ip) 
       {
      		m_remote_ip = ip;		  
       }  

	//
       void setDevGrpLast(E_GRP_FLAG grpflag) 
       {
      		 m_seGrplast = grpflag;
       }	
	void setCloseTime()
	{
		//m_bpendclose = true;
		m_closetime = tvNow();
	}
	
	std::string& getDevSbip()
	{
		return m_remote_ip;
	}
	std::string& getDevSbJqm()
	{
		return m_sbjqm;
	}
	E_GRP_FLAG getDevGrpLast() 
       {
      		 return m_seGrplast;
       }
	int pipe_write(void *pvBuf, unsigned int lBufSize)
	{
		return m_pipe.pipe_write(m_sbjqm,pvBuf, lBufSize);

	}
	int pipe_read(void *pvBuf, unsigned int lBufSize)
	{
		return m_pipe.pipe_read(m_sbjqm,pvBuf, lBufSize);
	}
	bool check_read_data(unsigned char* pdata)
	{
		return m_pipe.check_read_data(m_sbjqm,pdata);
	}

	bool check_pending_close()
	{
		return isClosed() && m_pipe.check_valid_data();	
	}
};

class Pipe : public CommsBase
{
protected:
	int mFd[2];

public:
	Pipe() : CommsBase( mFd[0], mFd[1] )
	{
		mFd[0] = -1;
		mFd[1] = -1;
	}
	~Pipe()
	{
        close();
	}

public:
	bool open();
	bool close();

	bool isOpen() const
	{
		return( mFd[0] != -1 && mFd[1] != -1 );
	}
	int getReadDesc() const
	{
		return( mFd[0] );
	}
	int getWriteDesc() const
	{
		return( mFd[1] );
	}

	bool setBlocking( bool blocking );
};

class SockAddr
{
private:
    const struct sockaddr *mAddr;

public:
    SockAddr( const struct sockaddr *addr );
    virtual ~SockAddr()
    {
    }

    static SockAddr *newSockAddr( const struct sockaddr &addr, socklen_t len );
    static SockAddr *newSockAddr( const SockAddr *addr );

    int getDomain() const
    {
        return( mAddr?mAddr->sa_family:AF_UNSPEC );
    }

    const struct sockaddr *getAddr() const
    {
        return( mAddr );
    }
    virtual socklen_t getAddrSize() const=0;
    virtual struct sockaddr *getTempAddr() const=0;
};

class SockAddrInet : public SockAddr
{
private:
    struct sockaddr_in  mAddrIn;
    struct sockaddr_in  mTempAddrIn;

public:
	SockAddrInet();
	SockAddrInet( const SockAddrInet &addr ) : SockAddr( (const struct sockaddr *)&mAddrIn ), mAddrIn( addr.mAddrIn )
    {
    }
	SockAddrInet( const struct sockaddr_in *addr ) : SockAddr( (const struct sockaddr *)&mAddrIn ), mAddrIn( *addr )
    {
    }


	bool resolve( const char *host, const char *serv, const char *proto );
	bool resolve( const char *host, int port, const char *proto );
	bool resolve( const char *serv, const char *proto );
	bool resolve( int port, const char *proto );

    socklen_t getAddrSize() const
    {
        return( sizeof(mAddrIn) );
    }
    struct sockaddr *getTempAddr() const
    {
        return( (sockaddr *)&mTempAddrIn );
    }
    struct sockaddr *getAddr() const
    {
        return( (sockaddr *)&mAddrIn );
    }   
public:
    static socklen_t addrSize()
    {
        return( sizeof(sockaddr_in) );
    }

};

class SockAddrUnix : public SockAddr
{
private:
    struct sockaddr_un  mAddrUn;
    struct sockaddr_un  mTempAddrUn;

public:
	SockAddrUnix();
	SockAddrUnix( const SockAddrUnix &addr ) : SockAddr( (const struct sockaddr *)&mAddrUn ), mAddrUn( addr.mAddrUn )
    {
    }
	SockAddrUnix( const struct sockaddr_un *addr ) : SockAddr( (const struct sockaddr *)&mAddrUn ), mAddrUn( *addr )
    {
    }

	bool resolve( const char *path, const char *proto );

    socklen_t getAddrSize() const
    {
        return( sizeof(mAddrUn) );
    }
    struct sockaddr *getTempAddr() const
    {
        return( (sockaddr *)&mTempAddrUn );
    }

public:
    static socklen_t addrSize()
    {
        return( sizeof(sockaddr_un) );
    }
};

class Socket : public CommsBase
{
protected:
    typedef enum { CLOSED, DISCONNECTED, LISTENING, CONNECTED } State;

protected:

    int	mSd;
    State mState;
    SockAddr *mLocalAddr;
    SockAddr *mRemoteAddr;

protected:
	Socket() : CommsBase( mSd, mSd ), mSd( -1 ), mState( CLOSED ), mLocalAddr( 0 ), mRemoteAddr( 0 )
	{
	}
	Socket( const Socket &socket, int newSd ) : CommsBase( mSd, mSd ), mSd( newSd ), mState( CONNECTED ), mLocalAddr( 0 ), mRemoteAddr( 0 )
	{
	        if ( socket.mLocalAddr )
	            mLocalAddr = SockAddr::newSockAddr( mLocalAddr );
	        if ( socket.mRemoteAddr )
	            mRemoteAddr = SockAddr::newSockAddr( mRemoteAddr );
	}
	virtual ~Socket()
	{
		close();
		delete mLocalAddr;
		delete mRemoteAddr;
	}

public:
    void mkSocketCons(const Socket &socket, int newSd)
    {
	mSd = newSd ;
	mState = CONNECTED;
	mLocalAddr=0;
	mRemoteAddr=0;
	if ( socket.mLocalAddr )
		mLocalAddr = SockAddr::newSockAddr( mLocalAddr );
	if ( socket.mRemoteAddr )
		mRemoteAddr = SockAddr::newSockAddr( mRemoteAddr );	
    }
    bool isOpen() const
    {
        return( !isClosed() );
    }
    bool isClosed() const
    {
        return( mState == CLOSED );
    }
    bool isDisconnected() const
    {
        return( mState == DISCONNECTED );
    }
    bool isConnected() const
    {
        return( mState == CONNECTED );
    }
	virtual bool close();

protected:
    bool isListening() const
    {
        return( mState == LISTENING );
    }

protected:
	virtual bool socket();
	virtual bool bind();

protected:
	virtual bool connect();
	virtual bool listen();
	virtual bool accept();
	virtual bool accept( int & );

public:
	virtual int send( const void *msg, int len ) const
	{
		ssize_t nBytes = ::send( mSd, msg, len, 0 );
        if ( nBytes < 0 )
            Debug( 1, "Send of %d bytes on sd %d failed: %s", len, mSd, strerror(errno) );
		return( nBytes );
	}
	virtual int recv( void *msg, int len ) const
	{
		ssize_t nBytes = ::recv( mSd, msg, len, 0 );
        if ( nBytes < 0 )
            Debug( 1, "Recv of %d bytes max on sd %d failed: %s", len, mSd, strerror(errno) );
		return( nBytes );
	}
	
	virtual int recv_nonblock( void *msg, int len ) const
	{
		ssize_t nBytes = ::recv( mSd, msg, len, MSG_DONTWAIT);
	        if ( nBytes < 0 )
	            Debug( 1, "recv_nonblock of %d bytes max on sd %d failed: %s", len, mSd, strerror(errno) );
			return( nBytes );
	}
	virtual int send_nonblock( const std::string &msg ) const
	{
		ssize_t tmp = 0;
		size_t total = msg.size();
		const char *p = msg.data();
		while(1)
		{
			tmp = ::send(mSd, p, total, 0);
			if(tmp < 0)
			{
				//if(errno == EINTR)
				//	return -1;
				if(errno == EAGAIN)
				{
					usleep(1000);
					continue;
				}
				Error("Send of string '%s' (%zd bytes) on sd %d failed: %s", msg.c_str(), msg.size(), mSd, strerror(errno));
				return -1;
			}
			if((size_t)tmp == total)
			{
				return  msg.size();
			}
			total -= tmp;
			p += tmp;
		}
		return tmp;
	}
	
	virtual int send( const std::string &msg ) const
	{
		ssize_t nBytes = ::send( mSd, msg.data(), msg.size(), 0 );
	        if ( nBytes < 0 )
	            Debug( 1, "Send of string '%s' (%zd bytes) on sd %d failed: %s", msg.c_str(), msg.size(), mSd, strerror(errno) );
			return( nBytes );
	}
	virtual int recv( std::string &msg ) const
	{
        char buffer[msg.capacity()];
        int nBytes = 0;
		if ( (nBytes = ::recv( mSd, buffer, sizeof(buffer), 0 )) < 0 )
        {
            Debug( 1, "Recv of %zd bytes max to string on sd %d failed: %s", sizeof(buffer), mSd, strerror(errno) );
            return( nBytes );
        }
        buffer[nBytes] = '\0';
        msg = buffer;
        return( nBytes );
	}
	virtual int recv( std::string &msg, size_t maxLen ) const
	{
        char buffer[maxLen];
        int nBytes = 0;
		if ( (nBytes = ::recv( mSd, buffer, sizeof(buffer), 0 )) < 0 )
        {
            Debug( 1, "Recv of %zd bytes max to string on sd %d failed: %s", maxLen, mSd, strerror(errno) );
            return( nBytes );
        }
        buffer[nBytes] = '\0';
        msg = buffer;
        return( nBytes );
	}
	virtual int bytesToRead() const;

	int getDesc() const
	{
		return( mSd );
	}
	//virtual bool isOpen() const
	//{
		//return( mSd != -1 );
	//}

    virtual int getDomain() const=0;
    virtual int getType() const=0;
    virtual const char *getProtocol() const=0;
    const SockAddr *getLocalAddr() const
    {
        return( mLocalAddr );
    }
    const SockAddr *getRemoteAddr() const
    {
        return( mRemoteAddr );
    }
    virtual socklen_t getAddrSize() const=0;

	bool getBlocking( bool &blocking );
	bool setBlocking( bool blocking );

	bool getSendBufferSize( int & ) const;
	bool getRecvBufferSize( int & ) const;

	bool setSendBufferSize( int );
	bool setRecvBufferSize( int );

	bool getRouting( bool & ) const;
	bool setRouting( bool );

	bool getNoDelay( bool & ) const;
	bool setNoDelay( bool );
};


class InetSocket : virtual public Socket
{
public:
    int getDomain() const
    {
        return( AF_INET );
    }
    virtual socklen_t getAddrSize() const
    {
        return( SockAddrInet::addrSize() );
    }

protected:
    bool resolveLocal( const char *host, const char *serv, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mLocalAddr = addr;
        return( addr->resolve( host, serv, proto ) );
    }
    bool resolveLocal( const char *host, int port, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mLocalAddr = addr;
        return( addr->resolve( host, port, proto ) );
    }
    bool resolveLocal( const char *serv, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mLocalAddr = addr;
        return( addr->resolve( serv, proto ) );
    }
    bool resolveLocal( int port, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mLocalAddr = addr;
        return( addr->resolve( port, proto ) );
    }

    bool resolveRemote( const char *host, const char *serv, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mRemoteAddr = addr;
        return( addr->resolve( host, serv, proto ) );
    }
    bool resolveRemote( const char *host, int port, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mRemoteAddr = addr;
        return( addr->resolve( host, port, proto ) );
    }

protected:
    bool bind( const SockAddrInet &addr )
    {
        mLocalAddr = new SockAddrInet( addr );
        return( Socket::bind() );
    }
    bool bind( const char *host, const char *serv ) 
    {
        if ( !resolveLocal( host, serv, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }
    bool bind( const char *host, int port )
    {
        if ( !resolveLocal( host, port, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }
    bool bind( const char *serv )
    {
        if ( !resolveLocal( serv, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }
    bool bind( int port )
    {
        if ( !resolveLocal( port, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }

    bool connect( const SockAddrInet &addr )
    {
        mRemoteAddr = new SockAddrInet( addr );
        return( Socket::connect() );
    }
    bool connect( const char *host, const char *serv )
    {
        if ( !resolveRemote( host, serv, getProtocol() ) )
            return( false );
        return( Socket::connect() );
    }
    bool connect( const char *host, int port )
    {
        if ( !resolveRemote( host, port, getProtocol() ) )
            return( false );
        return( Socket::connect() );
    }
};

class UnixSocket : virtual public Socket
{
public:
    int getDomain() const
    {
        return( AF_UNIX );
    }
    virtual socklen_t getAddrSize() const
    {
        return( SockAddrUnix::addrSize() );
    }

protected:
    bool resolveLocal( const char *serv, const char *proto )
    {
        SockAddrUnix *addr = new SockAddrUnix;
        mLocalAddr = addr;
        return( addr->resolve( serv, proto ) );
    }

    bool resolveRemote( const char *path, const char *proto )
    {
        SockAddrUnix *addr = new SockAddrUnix;
        mRemoteAddr = addr;
        return( addr->resolve( path, proto ) );
    }

protected:
    bool bind( const char *path )
    {
        if ( !UnixSocket::resolveLocal( path, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }

    bool connect( const char *path )
    {
        if ( !UnixSocket::resolveRemote( path, getProtocol() ) )
            return( false );
        return( Socket::connect() );
    }
};

class UdpSocket : virtual public Socket
{
public:
    int getType() const
    {
        return( SOCK_DGRAM );
    }
    const char *getProtocol() const
    {
        return( "udp" );
    }

public:
	virtual int sendto( const void *msg, int len, const SockAddr *addr=0 ) const
	{
		ssize_t nBytes = ::sendto( mSd, msg, len, 0, addr?addr->getAddr():NULL, addr?addr->getAddrSize():0 );
        if ( nBytes < 0 )
            Debug( 1, "Sendto of %d bytes on sd %d failed: %s", len, mSd, strerror(errno) );
		return( nBytes );
	}
	virtual int recvfrom( void *msg, int len, SockAddr **addr=0 ) const
	{
        ssize_t nBytes = 0;
        if ( addr )
        {
            struct sockaddr sockAddr;
            socklen_t sockLen;
		    nBytes = ::recvfrom( mSd, msg, len, 0, &sockAddr, &sockLen );
            if ( nBytes < 0 )
            {
                Debug( 1, "Recvfrom of %d bytes max on sd %d (with address) failed: %s", len, mSd, strerror(errno) );
            }
            else if ( sockLen )
            {
                *addr = SockAddr::newSockAddr( sockAddr, sockLen );
            }
        }   
        else
        {
		    nBytes = ::recvfrom( mSd, msg, len, 0, NULL, 0 );
            if ( nBytes < 0 )
                Debug( 1, "Recvfrom of %d bytes max on sd %d (no address) failed: %s", len, mSd, strerror(errno) );
        }
        return( nBytes );
	}
};

class UdpInetSocket : virtual public UdpSocket, virtual public InetSocket
{
public:
    bool bind( const SockAddrInet &addr ) 
    {
        return( InetSocket::bind( addr ) );
    }
    bool bind( const char *host, const char *serv ) 
    {
        return( InetSocket::bind( host, serv ) );
    }
    bool bind( const char *host, int port )
    {
        return( InetSocket::bind( host, port ) );
    }
    bool bind( const char *serv )
    {
        return( InetSocket::bind( serv ) );
    }
    bool bind( int port )
    {
        return( InetSocket::bind( port ) );
    }

    bool connect( const SockAddrInet &addr ) 
    {
        return( InetSocket::connect( addr ) );
    }
    bool connect( const char *host, const char *serv )
    {
        return( InetSocket::connect( host, serv ) );
    }
    bool connect( const char *host, int port )
    {
        return( InetSocket::connect( host, port ) );
    }
};

class UdpUnixSocket : virtual public UdpSocket, virtual public UnixSocket
{
public:
    bool bind( const char *path )
    {
        return( UnixSocket::bind( path ) );
    }

    bool connect( const char *path )
    {
        return( UnixSocket::connect( path ) );
    }
};

class UdpInetClient : public UdpInetSocket
{
protected:
    bool bind( const SockAddrInet &addr ) 
    {
        return( UdpInetSocket::bind( addr ) );
    }
    bool bind( const char *host, const char *serv )
    {
        return( UdpInetSocket::bind( host, serv ) );
    }
    bool bind( const char *host, int port )
    {
        return( UdpInetSocket::bind( host, port ) );
    }
    bool bind( const char *serv )
    {
        return( UdpInetSocket::bind( serv ) );
    }
    bool bind( int port )
    {
        return( UdpInetSocket::bind( port ) );
    }

public:
    bool connect( const SockAddrInet &addr ) 
    {
        return( UdpInetSocket::connect( addr ) );
    }
    bool connect( const char *host, const char *serv )
    {
        return( UdpInetSocket::connect( host, serv ) );
    }
    bool connect( const char *host, int port )
    {
        return( UdpInetSocket::connect( host, port ) );
    }
};

class UdpUnixClient : public UdpUnixSocket
{
public:
    bool bind( const char *path )
    {
        return( UdpUnixSocket::bind( path ) );
    }

public:
    bool connect( const char *path )
    {
        return( UdpUnixSocket::connect( path) );
    }
};

class UdpInetServer : public UdpInetSocket
{
public:
    bool bind( const SockAddrInet &addr ) 
    {
        return( UdpInetSocket::bind( addr ) );
    }
    bool bind( const char *host, const char *serv )
    {
        return( UdpInetSocket::bind( host, serv ) );
    }
    bool bind( const char *host, int port )
    {
        return( UdpInetSocket::bind( host, port ) );
    }
    bool bind( const char *serv )
    {
        return( UdpInetSocket::bind( serv ) );
    }
    bool bind( int port )
    {
        return( UdpInetSocket::bind( port ) );
    }

protected:
    bool connect( const char *host, const char *serv )
    {
        return( UdpInetSocket::connect( host, serv ) );
    }
    bool connect( const char *host, int port )
    {
        return( UdpInetSocket::connect( host, port ) );
    }
};

class UdpUnixServer : public UdpUnixSocket
{
public:
    bool bind( const char *path )
    {
        return( UdpUnixSocket::bind( path ) );
    }

protected:
    bool connect( const char *path )
    {
        return( UdpUnixSocket::connect( path ) );
    }
};

class TcpSocket : virtual public Socket
{
public:
    TcpSocket()
    {
    }
    TcpSocket( const TcpSocket &socket, int newSd ) : Socket( socket, newSd )
    {
    }

public:
    int getType() const
    {
        return( SOCK_STREAM );
    }
    const char *getProtocol() const
    {
        return( "tcp" );
    }
};

class TcpInetSocket : virtual public TcpSocket,virtual public InetSocket
{
public:
    TcpInetSocket()
    {
    }
    TcpInetSocket( const TcpInetSocket &socket, int newSd ) : TcpSocket( socket, newSd )
    {
	mkSocketCons(socket, newSd );
    }
};

class TcpUnixSocket : virtual public TcpSocket, virtual public UnixSocket
{
public:
    TcpUnixSocket()
    {
    }
    TcpUnixSocket( const TcpUnixSocket &socket, int newSd ) : TcpSocket( socket, newSd )
    {
    }
};

class TcpInetClient : public TcpInetSocket
{
public:
    bool connect( const char *host, const char *serv )
    {
        return( TcpInetSocket::connect( host, serv ) );
    }
    bool connect( const char *host, int port )
    {
        return( TcpInetSocket::connect( host, port ) );
    }
};

class TcpUnixClient : public TcpUnixSocket
{
public:
    bool connect( const char *path )
    {
        return( TcpUnixSocket::connect( path) );
    }
};

class TcpInetServer : public TcpInetSocket
{
public:
    bool bind( const char *host, const char *serv )
    {
        return( TcpInetSocket::bind( host, serv ) );
    }
    bool bind( const char *host, int port )
    {
        return( TcpInetSocket::bind( host, port ) );
    }
    bool bind( const char *serv )
    {
        return( TcpInetSocket::bind( serv ) );
    }
    bool bind( int port )
    {
        return( TcpInetSocket::bind( port ) );
    }

public:
    bool isListening() const { return( Socket::isListening() ); }
    bool listen();
    bool accept();
    bool accept( TcpInetSocket *&newSocket );
};

class TcpUnixServer : public TcpUnixSocket
{
public:
    bool bind( const char *path )
    {
        return( TcpUnixSocket::bind( path ) );
    }

public:
    bool isListening() const { return( Socket::isListening() ); }
    bool listen();
    bool accept();
    bool accept( TcpUnixSocket *&newSocket );
};


#ifdef EPOOL
#define MAXEPOLLSIZE 1024
class Epoll
{
    public:
        Epoll();
        bool Add(int fd,int eventsOption);
        //Returns the number of triggered events
        int Wait();
        bool Delete(const int eventIndex);
	 bool DeleteFd(int fd);
        int GetEventOccurfd(const int eventIndex) const;
        int GetEvents(const int eventIndex) const;

    private:
        int epollfd;
        int fdNumber;
        struct epoll_event event;
        struct epoll_event events[MAXEPOLLSIZE];
        struct rlimit rt;
};
#endif 

class Select
{
public:
    typedef std::set<CommsBase *> CommsSet;
    typedef std::vector<CommsBase *> CommsList;

protected:
    CommsSet        mReaders;
    CommsSet        mWriters;

    CommsList       mReadable;
    CommsList       mWriteable;

    CommsList       mPendClose;
	
    bool            mHasTimeout;
    struct timeval  mTimeout;
    int             mMaxFd;
    //listenfd
    CommsBase* mLsnFd;
#ifdef EPOOL
    Epoll m_epoll;
#endif

public:
    Select();
    Select( struct timeval timeout );
    Select( int timeout );
    Select( double timeout );

    void setTimeout( int timeout );
    void setTimeout( double timeout );
    void setTimeout( struct timeval timeout );
    void clearTimeout();

    void calcMaxFd();

    bool addReader( CommsBase *comms );
    bool deleteReader( CommsBase *comms );
    void clearReaders();

    bool addWriter( CommsBase *comms );
    bool deleteWriter( CommsBase *comms );
    void clearWriters();

    int wait();


    void addPendClose( CommsBase *comms );
    CommsList::iterator  deletePendClose( CommsList::iterator iter);	
    CommsList &getPendClose();

	
    const CommsList &getReadable() const;
    const CommsList &getWriteable() const;
    const CommsSet &getReaders() const;
    const CommsSet &getWriters() const;

    //for tcp
    int waitLsn();
    void setLsnFd(CommsBase *comms);
    bool addReaderLsn( CommsBase *comms );
    bool deleteReaderLsn( CommsBase *comms );
};


#endif // COMMS_H
