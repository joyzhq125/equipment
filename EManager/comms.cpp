



#include "logger.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
//#include <memory.h>
#include <alloca.h>
#include <string.h>
//#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/param.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>


#include "comms.h"
#include "db.h"

int CommsBase::readV( int iovcnt, /* const void *, int, */ ... )
{
	va_list arg_ptr;
	//struct iovec iov[iovcnt];
	struct iovec *iov = (struct iovec *)alloca( sizeof(struct iovec)*iovcnt );

	va_start( arg_ptr, iovcnt );
	for ( int i = 0; i < iovcnt; i++ )
	{
		iov[i].iov_base = va_arg( arg_ptr, void * );
		iov[i].iov_len = va_arg( arg_ptr, int );
	}
	va_end( arg_ptr );

    int nBytes = ::readv( mRd, iov, iovcnt );
    if ( nBytes < 0 )
        Debug( 1, "Readv of %d buffers max on rd %d failed: %s", iovcnt, mRd, strerror(errno) );
    return( nBytes );
}

int CommsBase::writeV( int iovcnt, /* const void *, int, */ ... )
{
	va_list arg_ptr;
	//struct iovec iov[iovcnt];
	struct iovec *iov = (struct iovec *)alloca( sizeof(struct iovec)*iovcnt );

	va_start( arg_ptr, iovcnt );
	for ( int i = 0; i < iovcnt; i++ )
	{
		iov[i].iov_base = va_arg( arg_ptr, void * );
		iov[i].iov_len = va_arg( arg_ptr, int );
	}
	va_end( arg_ptr );

    ssize_t nBytes = ::writev( mWd, iov, iovcnt );
    if ( nBytes < 0 )
        Debug( 1, "Writev of %d buffers on wd %d failed: %s", iovcnt, mWd, strerror(errno) );
    return( nBytes );
}

bool Pipe::open()
{
	if ( ::pipe( mFd ) < 0 )
	{
		Error( "pipe(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}

	return( true );
}

bool Pipe::close()
{
	if ( mFd[0] > -1 ) ::close( mFd[0] );
	mFd[0] = -1;
	if ( mFd[1] > -1 ) ::close( mFd[1] );
	mFd[1] = -1;
	return( true );
}

bool Pipe::setBlocking( bool blocking )
{
	int flags;

	/* Now set it for non-blocking I/O */
	if ( (flags = fcntl( mFd[1], F_GETFL )) < 0 )
	{
		Error( "fcntl(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	if ( blocking )
	{
		flags &= ~O_NONBLOCK;
	}
	else
	{
		flags |= O_NONBLOCK;
	}
	if ( fcntl( mFd[1], F_SETFL, flags ) < 0 )
	{
		Error( "fcntl(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}

	return( true );
}

SockAddr::SockAddr( const struct sockaddr *addr ) : mAddr( addr )
{
}

SockAddr *SockAddr::newSockAddr( const struct sockaddr &addr, socklen_t len )
{
    if ( addr.sa_family == AF_INET && len == SockAddrInet::addrSize() )
    {
        return( new SockAddrInet( (const struct sockaddr_in *)&addr ) );
    }
    else if ( addr.sa_family == AF_UNIX && len == SockAddrUnix::addrSize() )
    {
        return( new SockAddrUnix( (const struct sockaddr_un *)&addr ) );
    }
    Error( "Unable to create new SockAddr from addr family %d with size %d", addr.sa_family, len );
    return( 0 );
}

SockAddr *SockAddr::newSockAddr( const SockAddr *addr )
{
    if ( !addr )
        return( 0 );

    if ( addr->getDomain() == AF_INET )
    {
        return( new SockAddrInet( *(SockAddrInet *)addr ) );
    }
    else if ( addr->getDomain() == AF_UNIX )
    {
        return( new SockAddrUnix( *(SockAddrUnix *)addr ) );
    }
    Error( "Unable to create new SockAddr from addr family %d", addr->getDomain() );
    return( 0 );
}

SockAddrInet::SockAddrInet() : SockAddr( (struct sockaddr *)&mAddrIn )
{
}

bool SockAddrInet::resolve( const char *host, const char *serv, const char *proto )
{
    memset( &mAddrIn, 0, sizeof(mAddrIn) );

    struct hostent *hostent=0;
    if ( !(hostent = ::gethostbyname( host ) ) )
    {
        Error( "gethostbyname( %s ), h_errno = %d", host, h_errno );
        return( false );
    }

    struct servent *servent=0;
    if ( !(servent = ::getservbyname( serv, proto ) ) )
    {
        Error( "getservbyname( %s ), errno = %d, error = %s", serv, errno, strerror(errno) );
        return( false );
    }

    mAddrIn.sin_port = servent->s_port;
    mAddrIn.sin_family = AF_INET;
    mAddrIn.sin_addr.s_addr = ((struct in_addr *)(hostent->h_addr))->s_addr;

    return( true );
}

bool SockAddrInet::resolve( const char *host, int port, const char *proto )
{
    memset( &mAddrIn, 0, sizeof(mAddrIn) );

    struct hostent *hostent=0;
    if ( !(hostent = ::gethostbyname( host ) ) )
    {
        Error( "gethostbyname( %s ), h_errno = %d", host, h_errno );
        return( false );
    }

    mAddrIn.sin_port = htons(port);
    mAddrIn.sin_family = AF_INET;
    mAddrIn.sin_addr.s_addr = ((struct in_addr *)(hostent->h_addr))->s_addr;
    return( true );
}

bool SockAddrInet::resolve( const char *serv, const char *proto )
{
    memset( &mAddrIn, 0, sizeof(mAddrIn) );

    struct servent *servent=0;
    if ( !(servent = ::getservbyname( serv, proto ) ) )
    {
        Error( "getservbyname( %s ), errno = %d, error = %s", serv, errno, strerror(errno) );
        return( false );
    }

    mAddrIn.sin_port = servent->s_port;
    mAddrIn.sin_family = AF_INET;
	mAddrIn.sin_addr.s_addr = INADDR_ANY;

    return( true );
}

bool SockAddrInet::resolve( int port, const char *proto )
{
    memset( &mAddrIn, 0, sizeof(mAddrIn) );

    mAddrIn.sin_port = htons(port);
    mAddrIn.sin_family = AF_INET;
	mAddrIn.sin_addr.s_addr = INADDR_ANY;

    return( true );
}

SockAddrUnix::SockAddrUnix() : SockAddr( (struct sockaddr *)&mAddrUn )
{
}

bool SockAddrUnix::resolve( const char *path, const char *proto )
{
    memset( &mAddrUn, 0, sizeof(mAddrUn) );

    strncpy( mAddrUn.sun_path, path, sizeof(mAddrUn.sun_path) );
    mAddrUn.sun_family = AF_UNIX;

    return( true );
}

bool Socket::socket()
{
    if ( mSd >= 0 )
        return( true );

	if ( (mSd = ::socket( getDomain(), getType(), 0 ) ) < 0 )
	{
		Error( "socket(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}

	int val = 1;

	(void)::setsockopt( mSd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val) );
	(void)::setsockopt( mSd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val) );

       mState = DISCONNECTED;

	return( true );
}

bool Socket::connect()
{
	if ( !socket() ) 
		return( false );

	if ( ::connect( mSd, mRemoteAddr->getAddr(), getAddrSize() ) == -1 )
	{
		Error( "connect(), errno = %d, error = %s", errno, strerror(errno) );
		close();
		return( false );
	}

    mState = CONNECTED;

	return( true );
}

bool Socket::bind()
{
	if ( !socket() )
		return( false );

	if ( ::bind( mSd, mLocalAddr->getAddr(), getAddrSize() ) == -1 )
	{
		Error( "bind(), errno = %d, error = %s", errno, strerror(errno) );
		close();
		return( false );
	}
    return( true );
}

bool Socket::listen()
{
	if ( ::listen( mSd, SOMAXCONN ) == -1 )
	{
		Error( "listen(), errno = %d, error = %s", errno, strerror(errno) );
		close();
		return( false );
	}

    mState = LISTENING;

	return( true );
}

bool Socket::accept()
{
    struct sockaddr *rem_addr = mLocalAddr->getTempAddr();
    socklen_t rem_addr_size = getAddrSize();

    int newSd = -1;
    if ( (newSd = ::accept( mSd, rem_addr, &rem_addr_size )) == -1 )
    {
        Error( "accept(), errno = %d, error = %s", errno, strerror(errno) );
        close();
        return( false );
    }

	::close( mSd );
	mSd = newSd;

    mState = CONNECTED;

	return( true );
}

bool Socket::accept( int &newSd )
{
    struct sockaddr *rem_addr = mLocalAddr->getTempAddr();
    socklen_t rem_addr_size = getAddrSize();

    newSd = -1;
    if ( (newSd = ::accept( mSd, rem_addr, &rem_addr_size )) == -1 )
    {
        Error( "accept(), errno = %d, error = %s", errno, strerror(errno) );
        close();
        return( false );
    }

	return( true );
}

bool Socket::close()
{
	if ( mSd > -1 ) ::close( mSd );
	mSd = -1;
        mState = CLOSED;
	return( true );
}

int Socket::bytesToRead() const
{
	int bytes_to_read = 0;

	if ( ioctl( mSd, FIONREAD, &bytes_to_read ) < 0 )
	{
		Error( "ioctl(), errno = %d, error = %s", errno, strerror(errno) );
		return( -1 );
	}
	return( bytes_to_read );
}

bool Socket::getBlocking( bool &blocking )
{
	int flags;

	if ( (flags = fcntl( mSd, F_GETFL )) < 0 )
	{
		Error( "fcntl(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	blocking = (flags & O_NONBLOCK);
	return( true );
}

bool Socket::setBlocking( bool blocking )
{
#if 0
	// ioctl is apparently not recommended
	int ioctl_arg = !blocking;
	if ( ioctl( mSd, FIONBIO, &ioctl_arg ) < 0 )
	{
		Error( "ioctl(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	return( true );
#endif

	int flags;

	/* Now set it for non-blocking I/O */
	if ( (flags = fcntl( mSd, F_GETFL )) < 0 )
	{
		Error( "fcntl(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	if ( blocking )
	{
		flags &= ~O_NONBLOCK;
	}
	else
	{
		flags |= O_NONBLOCK;
	}
	if ( fcntl( mSd, F_SETFL, flags ) < 0 )
	{
		Error( "fcntl(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}

	return( true );
}

bool Socket::getSendBufferSize( int &buffersize ) const
{
	socklen_t optlen = sizeof(buffersize);
	if ( getsockopt( mSd, SOL_SOCKET, SO_SNDBUF, &buffersize, &optlen ) < 0 )
	{
		Error( "getsockopt(), errno = %d, error = %s", errno, strerror(errno) );
		return( -1 );
	}
	return( buffersize );
}

bool Socket::getRecvBufferSize( int &buffersize ) const
{
	socklen_t optlen = sizeof(buffersize);
	if ( getsockopt( mSd, SOL_SOCKET, SO_RCVBUF, &buffersize, &optlen ) < 0 )
	{
		Error( "getsockopt(), errno = %d, error = %s", errno, strerror(errno) );
		return( -1 );
	}
	return( buffersize );
}

bool Socket::setSendBufferSize( int buffersize )
{
	if ( setsockopt( mSd, SOL_SOCKET, SO_SNDBUF, (char *)&buffersize, sizeof(buffersize)) < 0 )
	{
		Error( "setsockopt(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	return( true );
}

bool Socket::setRecvBufferSize( int buffersize )
{
	if ( setsockopt( mSd, SOL_SOCKET, SO_RCVBUF, (char *)&buffersize, sizeof(buffersize)) < 0 )
	{
		Error( "setsockopt(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	return( true );
}

bool Socket::getRouting( bool &route ) const
{
	int dontRoute;
	socklen_t optlen = sizeof(dontRoute);
	if ( getsockopt( mSd, SOL_SOCKET, SO_DONTROUTE, &dontRoute, &optlen ) < 0 )
	{
		Error( "getsockopt(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	route = !dontRoute;
	return( true );
}

bool Socket::setRouting( bool route )
{
	int dontRoute = !route;
	if ( setsockopt( mSd, SOL_SOCKET, SO_DONTROUTE, (char *)&dontRoute, sizeof(dontRoute)) < 0 )
	{
		Error( "setsockopt(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	return( true );
}

bool Socket::getNoDelay( bool &nodelay ) const
{
	int int_nodelay;
	socklen_t optlen = sizeof(int_nodelay);
	if ( getsockopt( mSd, IPPROTO_TCP, TCP_NODELAY, &int_nodelay, &optlen ) < 0 )
	{
		Error( "getsockopt(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	nodelay = int_nodelay;
	return( true );
}

bool Socket::setNoDelay( bool nodelay )
{
	int int_nodelay = nodelay;

	if ( setsockopt( mSd, IPPROTO_TCP, TCP_NODELAY, (char *)&int_nodelay, sizeof(int_nodelay)) < 0 )
	{
		Error( "setsockopt(), errno = %d, error = %s", errno, strerror(errno) );
		return( false );
	}
	return( true );
}

bool TcpInetServer::listen()
{
    return( Socket::listen() );
}

bool TcpInetServer::accept()
{
    return( Socket::accept() );
}

bool TcpInetServer::accept( TcpInetSocket *&newSocket )
{
    int newSd = -1;
    newSocket = 0;
    if ( !Socket::accept( newSd ) )
        return( false );
     Info("TcpInetServer::accept sockid %d",newSd);
     newSocket = new TcpInetSocket( *this, newSd );
     return( true );
}


bool TcpUnixServer::accept( TcpUnixSocket *&newSocket )
{
    int newSd = -1;
    newSocket = 0;

    if ( !Socket::accept( newSd ) )
        return( false );

	newSocket = new TcpUnixSocket( *this, newSd );

    return( true );
}

Select::Select() : mHasTimeout( false ), mMaxFd( -1 )
{
}

Select::Select( struct timeval timeout ) : mMaxFd( -1 )
{
    setTimeout( timeout );
}

Select::Select( int timeout ) : mMaxFd( -1 )
{
    setTimeout( timeout );
}

Select::Select( double timeout ) : mMaxFd( -1 )
{
    setTimeout( timeout );
}

void Select::setTimeout( int timeout )
{
    mTimeout.tv_sec = timeout;
    mTimeout.tv_usec = 0;
    mHasTimeout = true;
}

void Select::setTimeout( double timeout )
{
    mTimeout.tv_sec = int(timeout);
    mTimeout.tv_usec = suseconds_t((timeout-mTimeout.tv_sec)*1000000.0);
    mHasTimeout = true;
}

void Select::setTimeout( struct timeval timeout )
{
    mTimeout = timeout;
    mHasTimeout = true;
}

void Select::clearTimeout()
{
    mHasTimeout = false;
}

void Select::calcMaxFd()
{
    mMaxFd = -1;
    for ( CommsSet::iterator iter = mReaders.begin(); iter != mReaders.end(); iter++ )
        if ( (*iter)->getMaxDesc() > mMaxFd )
            mMaxFd = (*iter)->getMaxDesc();
    for ( CommsSet::iterator iter = mWriters.begin(); iter != mWriters.end(); iter++ )
        if ( (*iter)->getMaxDesc() > mMaxFd )
            mMaxFd = (*iter)->getMaxDesc();
}

bool Select::addReaderLsn( CommsBase *comms )
{
    if ( !comms->isOpen() )
    {
        Error( "Unable to add closed reader" );
        return( false );
    }
		
    std::pair<CommsSet::iterator,bool> result = mReaders.insert( comms );
#ifdef EPOOL	
    if(mLsnFd->getReadDesc() != comms->getReadDesc() && !m_epoll.Add(comms->getReadDesc(),EPOLLIN))
		return false;
#endif 
    if ( result.second )
        if ( comms->getMaxDesc() > mMaxFd )
            mMaxFd = comms->getMaxDesc();
    return( result.second );
}

bool Select::deleteReaderLsn( CommsBase *comms )
{
    if ( !comms->isOpen() )
    {
        Error( "Unable to add closed reader" );
        return( false );
    }
    if ( mReaders.erase( comms ) )
    {
        calcMaxFd();
#ifdef EPOOL	
	 m_epoll.DeleteFd(comms->getReadDesc());
#endif 
        return( true );
    }
    return( false );
}

bool Select::addReader( CommsBase *comms )
{
    if ( !comms->isOpen() )
    {
        Error( "Unable to add closed reader" );
        return( false );
    }
		
    std::pair<CommsSet::iterator,bool> result = mReaders.insert( comms );
    if ( result.second )
        if ( comms->getMaxDesc() > mMaxFd )
            mMaxFd = comms->getMaxDesc();
    return( result.second );
}

bool Select::deleteReader( CommsBase *comms )
{
    if ( !comms->isOpen() )
    {
        Error( "Unable to add closed reader" );
        return( false );
    }
    if ( mReaders.erase( comms ) )
    {
        calcMaxFd();
        return( true );
    }
    return( false );
}

void Select::clearReaders()
{
    mReaders.clear();
    mMaxFd = -1;
}

bool Select::addWriter( CommsBase *comms )
{
    std::pair<CommsSet::iterator,bool> result = mWriters.insert( comms );
    if ( result.second )
        if ( comms->getMaxDesc() > mMaxFd )
            mMaxFd = comms->getMaxDesc();
    return( result.second );
}

bool Select::deleteWriter( CommsBase *comms )
{
    if ( mWriters.erase( comms ) )
    {
        calcMaxFd();
        return( true );
    }
    return( false );
}

void Select::clearWriters()
{
    mWriters.clear();
    mMaxFd = -1;
}

void Select::setLsnFd(CommsBase *comms) 
{
	mLsnFd = comms;
	
  	if(!m_epoll.Add(comms->getReadDesc(),EPOLLIN))
       {
       	Error("Select::setLsnFd add error");
		return;
  	}
  	
}
int Select::waitLsn()
{
	struct timeval tempTimeout = mTimeout;
	struct timeval *selectTimeout = mHasTimeout?&tempTimeout:NULL;
#ifdef EPOOL
	int eventNumber = 0;
	int idx = 0;
	
	mReadable.clear();
	/*
	for ( CommsSet::iterator iter = mReaders.begin(); iter != mReaders.end(); iter++ )
	{
		if(!m_epoll.Add((*iter)->getReadDesc(),EPOLLIN | EPOLLET))
			return 0;
	}
	*/
	eventNumber = m_epoll.Wait();
	if( eventNumber == 0 )
	{
		Debug( 1, "epoll Wait timed out" );
	}
	else if(eventNumber < 0)
	{
		
		if(errno ==  EINTR) 
		{
			Error( "epoll Wait EINTR error: %s", strerror(errno));
		}            
		else                                              
		{   
		       Error( "epoll Wait other error: %s", strerror(errno)); 
		}                                
		
	}
	else
	{
		TcpInetServer *socket = dynamic_cast<TcpInetServer *>(mLsnFd);
		struct sockaddr_in* paddr_srv =  (struct sockaddr_in*)socket->getLocalAddr()->getAddr();	
		Info("Select::waitLsn port %u new data coming ins,readers num is %d ",ntohs(paddr_srv->sin_port),mReaders.size());
		 for(idx = 0;idx < eventNumber;idx++ )
	        {
	             if ((m_epoll.GetEvents(idx) & EPOLLERR) ||
	                 (m_epoll.GetEvents(idx) & EPOLLHUP) ||
	                 (!(m_epoll.GetEvents(idx) & EPOLLIN)))
	             {
	                      /* An error has occured on this fd, or the socket is not
	                         ready for reading (why were we notified then?) */
	                     Error ("epoll error sock id %d",m_epoll.GetEventOccurfd(idx));
	                      //DeleteClient(epoll.GetEventOccurfd(i));
				for ( CommsSet::iterator iter = mReaders.begin(); iter != mReaders.end(); iter++ )
				{
					if(m_epoll.GetEventOccurfd(idx) == (*iter)->getReadDesc())
					{
						Info("find sockid %d and close it",m_epoll.GetEventOccurfd(idx));
						TcpInetSocket *socket = dynamic_cast<TcpInetSocket *>(*iter);
						deleteReaderLsn(*iter);
						socket->close();
						addPendClose(socket);
						socket->setCloseTime();
						//delete socket;
						break;
					}
				}						  
	                     continue;
	             }
	            //if event is triggered by listener socket
	            else if(m_epoll.GetEventOccurfd(idx) == mLsnFd->getReadDesc())
	            {					
				Debug(2,"Select::waitLsn new connect");
				/*CommsBase* connfd = NULL;*/
				TcpInetSocket * connfd = NULL;
				if(!socket->accept(connfd) && connfd)
				{
					/*
					if(errno ==  EINTR) 
					{

					}            
					else                                              
					{   
					          
					}                                
					*/
					Error( "accept error: %s", strerror(errno) );
					return 0;
				}
				//add new connect
				struct sockaddr_in* paddr_conn = (struct sockaddr_in*)socket->getLocalAddr()->getTempAddr();
				std::string conn_ip = inet_ntoa(paddr_conn->sin_addr);

				Info("connect ip is %s,port is %u",conn_ip.c_str(),ntohs(paddr_conn->sin_port));
				//ip address
				connfd->setDevSbip(conn_ip);
				addReaderLsn(connfd);
				connfd->setBlocking(false);		

				//命令主机上报设备全码	
				string cmd_devcode = (const char *)config.Item(g_dbconn,E_CFG_D_DEVCODE );		
				if(!cmd_devcode.empty() && connfd->send_nonblock(cmd_devcode) < 0)
				{
					Error("cannot send cmd devcode");
				}

	            }
		     else
		     {
				for ( CommsSet::iterator iter = mReaders.begin(); iter != mReaders.end(); iter++ )
					if ((*iter)->getReadDesc() == m_epoll.GetEventOccurfd(idx))
						mReadable.push_back( *iter );
		     }
	        }
	}
	return eventNumber;
#else
	fd_set rfds;
	fd_set wfds;

	mReadable.clear();
	FD_ZERO(&rfds);
	for ( CommsSet::iterator iter = mReaders.begin(); iter != mReaders.end(); iter++ )
		FD_SET((*iter)->getReadDesc(),&rfds);

	mWriteable.clear();
	FD_ZERO(&wfds);
	for ( CommsSet::iterator iter = mWriters.begin(); iter != mWriters.end(); iter++ )
		FD_SET((*iter)->getWriteDesc(),&wfds);

	int nFound = select( mMaxFd+1, &rfds, &wfds, NULL, selectTimeout );
	if( nFound == 0 )
	{
		Debug( 1, "Select timed out" );
	}
	else if ( nFound < 0)
	{   
	/*
	if(errno ==  EINTR) 
	{

	}            
	else                                              
	{   
	                      
	}                                
	*/
		Error( "Select error: %s", strerror(errno) );
	}
	else
	{
		TcpInetServer *socket = dynamic_cast<TcpInetServer *>(mLsnFd);
		struct sockaddr_in* paddr_srv =  (struct sockaddr_in*)socket->getLocalAddr()->getAddr();	
		Debug(2,"Select::waitLsn port %u new data coming ins,readers num is %d ",ntohs(paddr_srv->sin_port),mReaders.size());
		// check if listen fd
		if (mLsnFd && FD_ISSET(mLsnFd->getReadDesc(),&rfds))     
		{
			Debug(2,"Select::waitLsn new connect");
			/*CommsBase* connfd = NULL;*/
			TcpInetSocket * connfd = NULL;
			if(!socket->accept(connfd) && connfd)
			{
				/*
				if(errno ==  EINTR) 
				{

				}            
				else                                              
				{   
				                  
				}                                
				*/
				Error( "accept error: %s", strerror(errno) );
				return 0;
			}
			//add new connect
			struct sockaddr_in* paddr_conn = (struct sockaddr_in*)socket->getLocalAddr()->getTempAddr();
			std::string conn_ip = inet_ntoa(paddr_conn->sin_addr);

			Info("connect ip is %s,port is %u",conn_ip.c_str(),ntohs(paddr_conn->sin_port));
			//ip address
			connfd->setDevSbip(conn_ip);
			addReader(connfd);

		}	
		Info("Select::waitLsn port %u new data coming ine,readers num is %d ",ntohs(paddr_srv->sin_port),mReaders.size());
		for ( CommsSet::iterator iter = mReaders.begin(); iter != mReaders.end(); iter++ )
			if ( FD_ISSET((*iter)->getReadDesc(),&rfds) && (*iter)->getReadDesc() != mLsnFd->getReadDesc()) //except listen fd
			mReadable.push_back( *iter );
		for ( CommsSet::iterator iter = mWriters.begin(); iter != mWriters.end(); iter++ )
			if ( FD_ISSET((*iter)->getWriteDesc(),&rfds) )
			mWriteable.push_back( *iter );
	}

	return( nFound );
#endif

}
int Select::wait()
{
    struct timeval tempTimeout = mTimeout;
    struct timeval *selectTimeout = mHasTimeout?&tempTimeout:NULL;

    fd_set rfds;
    fd_set wfds;

    mReadable.clear();
    FD_ZERO(&rfds);
    for ( CommsSet::iterator iter = mReaders.begin(); iter != mReaders.end(); iter++ )
        FD_SET((*iter)->getReadDesc(),&rfds);

    mWriteable.clear();
    FD_ZERO(&wfds);
    for ( CommsSet::iterator iter = mWriters.begin(); iter != mWriters.end(); iter++ )
        FD_SET((*iter)->getWriteDesc(),&wfds);

    int nFound = select( mMaxFd+1, &rfds, &wfds, NULL, selectTimeout );
    if( nFound == 0 )
    {
        Debug( 1, "Select timed out" );
    }
    else if ( nFound < 0)
    {
        Error( "Select error: %s", strerror(errno) );
    }
    else
    {
        for ( CommsSet::iterator iter = mReaders.begin(); iter != mReaders.end(); iter++ )
            if ( FD_ISSET((*iter)->getReadDesc(),&rfds) )
                mReadable.push_back( *iter );
        for ( CommsSet::iterator iter = mWriters.begin(); iter != mWriters.end(); iter++ )
            if ( FD_ISSET((*iter)->getWriteDesc(),&rfds) )
                mWriteable.push_back( *iter );
    }
    return( nFound );
}

void  Select::addPendClose( CommsBase *comms )
{
    //std::pair<CommsSet::iterator,bool> result = mPendClose.insert( comms );
   // return( result.second );
    mPendClose.push_back(comms);
   //return true;
}

Select::CommsList::iterator Select::deletePendClose(Select::CommsList::iterator iter)
{
   return mPendClose.erase( iter );
}


  Select::CommsList &Select::getPendClose() 
 {
    return(mPendClose );
 }

const Select::CommsList &Select::getReadable() const
{
    return( mReadable );
}

const Select::CommsList &Select::getWriteable() const
{
    return( mWriteable );
}

const Select::CommsSet &Select::getReaders() const
{
    return( mReaders );
}

const Select::CommsSet &Select::getWriters() const
{
    return( mWriters );
}


#ifdef EPOOL
// epoll
Epoll::Epoll():fdNumber(0)
{
    //set resource limits respectively
    rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE;
    if(::setrlimit(RLIMIT_NOFILE, &rt) == -1)
    {
        Error("setrlimit");
    }
    //create epoll
    epollfd = epoll_create(MAXEPOLLSIZE);
}

bool Epoll::Add(int fd,int eventsOption)
{
    //handle readable event,set Edge Triggered
    event.events = eventsOption;//EPOLLIN | EPOLLET;
    event.data.fd = fd;

    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event)<0)
        return false;

    fdNumber++;
    return true;
}

bool Epoll::Delete(const int eventIndex)
{
    if(epoll_ctl(epollfd,EPOLL_CTL_DEL, events[eventIndex].data.fd,&event)<0)
        return false;
	
    fdNumber--;
    return true;
}

bool Epoll::DeleteFd(int fd)
{
    if(epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&event) < 0)
        return false;
	
    fdNumber--;
    return true;
}

int Epoll::Wait()
{
    int eventNumber = 0;
    eventNumber = epoll_wait(epollfd,events,fdNumber,-1);
    if(eventNumber < 0)
    {
        Error("epoll_wait");
    }
    return eventNumber;
}

int Epoll::GetEventOccurfd(const int eventIndex) const
{
    return events[eventIndex].data.fd;
}

int Epoll::GetEvents(const int eventIndex) const
{
    return events[eventIndex].events;
}

#endif
