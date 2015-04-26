#include "dc4c_util.h"

int ConvertToDaemonServer()
{
	int pid;
	int sig,fd;
	
	pid=fork();
	
	switch( pid )
	{
		case -1:
			return -1;
		case 0:
			break;
		default		:
			exit( 0 );	
			break;
	}

	setsid() ;
	signal( SIGHUP,SIG_IGN );

	pid=fork();

	switch( pid )
	{
		case -1:
			return -1;
		case 0:
			break ;
		default:
			exit( 0 );
			break;
	}
	
	setuid( getpid() ) ;
	
	chdir("/tmp");
	
	umask( 0 ) ;
	
	for( sig=0 ; sig<30 ; sig++ )
		signal( sig , SIG_IGN );
	
	signal( SIGTERM , SIG_DFL );
	
	for( fd=0 ; fd<=2 ; fd++ )
		close( fd );
	
	return 0;
}

int ns2i( char *str , int len )
{
	char	buf[ 10 + 1 ] ;
	
	memset( buf , 0x00 , sizeof(buf) );
	strncpy( buf , str , len );
	
	return atoi( buf ) ;
}

void CleanSocketSession( struct SocketSession *psession )
{
	if( psession )
	{
		if( psession->recv_buffer )
		{
			free( psession->recv_buffer );
		}
		
		if( psession->send_buffer )
		{
			free( psession->send_buffer );
		}
	}
	
	return;
}

int InitSocketSession( struct SocketSession *psession )
{
	memset( psession , 0x00 , sizeof(struct SocketSession) );
	
	psession->recv_buffer_size = 1024+1 ;
	psession->recv_buffer = (char*) malloc( psession->recv_buffer_size ) ;
	if( psession->recv_buffer == NULL )
	{
		CleanSocketSession( psession );
		return -1;
	}
	CleanSendBuffer( psession );
	
	psession->send_buffer_size = 1024+1 ;
	psession->send_buffer = (char*) malloc( psession->send_buffer_size ) ;
	if( psession->send_buffer == NULL )
	{
		CleanSocketSession( psession );
		return -1;
	}
	CleanRecvBuffer( psession );
	
	return 0;
}

void FreeSocketSession( struct SocketSession *psession )
{
	CleanSocketSession( psession );
	
	free( psession );
	
	return;
}

struct SocketSession *AllocSocketSession()
{
	struct SocketSession	*psession = NULL ;
	
	int			nret = 0 ;
	
	psession = (struct SocketSession *)malloc( sizeof(struct SocketSession) ) ;
	if( psession == NULL )
		return NULL;
	
	nret = InitSocketSession( psession ) ;
	if( nret )
	{
		FreeSocketSession( psession );
		return NULL;
	}
	
	return psession;
}

void ResetSocketSession( struct SocketSession *psession )
{
	memset( psession->ip , 0x00 , sizeof(psession->ip) );
	return;
}

void CleanSendBuffer( struct SocketSession *psession )
{
	memset( psession->send_buffer , 0x00 , psession->send_buffer_size );
	psession->total_send_len = 0 ;
	psession->send_len = 0 ;
	
	return;
}

void CleanRecvBuffer( struct SocketSession *psession )
{
	memset( psession->recv_buffer , 0x00 , psession->recv_buffer_size );
	psession->total_recv_len = 0 ;
	psession->recv_body_len = 0 ;
	
	return;
}

int ReallocSendBuffer( struct SocketSession *psession , int new_send_buffer_size )
{
	char		*new_send_buffer = NULL ;
	
	if( new_send_buffer_size <= psession->send_buffer_size )
		return 0;
	
	new_send_buffer = (char*) realloc( psession->send_buffer , new_send_buffer_size ) ;
	if( new_send_buffer == NULL )
		return -1;
	// memset( psession->send_buffer + psession->total_send_len , 0x00 , new_send_buffer_size - psession->total_send_len );
	
	psession->send_buffer = new_send_buffer ;
	psession->send_buffer_size = new_send_buffer_size ;
	
	return 0;
}

int ReallocRecvBuffer( struct SocketSession *psession , int new_recv_buffer_size )
{
	char		*new_recv_buffer = NULL ;
	
	if( new_recv_buffer_size <= psession->recv_buffer_size )
		return 0;
	
	new_recv_buffer = (char*) realloc( psession->recv_buffer , new_recv_buffer_size ) ;
	if( new_recv_buffer == NULL )
		return -1;
	// memset( psession->recv_buffer + psession->total_recv_len , 0x00 , new_recv_buffer_size - psession->total_recv_len );
	
	psession->recv_buffer = new_recv_buffer ;
	psession->recv_buffer_size = new_recv_buffer_size ;
	
	return 0;
}

struct SocketSession *GetUnusedSocketSession( struct SocketSession *p_session_array , int count , struct SocketSession **pp_slibing_session )
{
	struct SocketSession	*p_last_session = NULL ;
	int			i ;
	
	p_last_session = p_session_array + count - 1 ;
	
	for( i = 0 ; i < count ; i++ )
	{
		if( (*pp_slibing_session) == NULL )
		{
			(*pp_slibing_session) = p_session_array+0 ;
		}
		else
		{
			(*pp_slibing_session)++;
			if( (*pp_slibing_session) > p_last_session )
				(*pp_slibing_session) = p_session_array+0 ;
		}
		
		if( (*pp_slibing_session)->ip[0] == 0 )
		{
			return (*pp_slibing_session);
		}
	}
	
	return NULL;
}

int AddInputSockToEpoll( int epoll_socks , struct SocketSession *psession )
{
	struct epoll_event	event ;
	
	int			nret = 0 ;
	
	memset( & event , 0x00 , sizeof(event) );
	event.data.ptr = psession ;
	event.events = EPOLLIN | EPOLLERR ;
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_ADD , psession->sock , & event ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "add sock[%d] to wait EPOLLIN event failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "add sock[%d] to wait EPOLLIN event" , psession->sock );
		return 0;
	}
}

int AddOutputSockToEpoll( int epoll_socks , struct SocketSession *psession )
{
	struct epoll_event	event ;
	
	int			nret = 0 ;
	
	memset( & event , 0x00 , sizeof(event) );
	event.data.ptr = psession ;
	event.events = EPOLLOUT | EPOLLERR ;
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_ADD , psession->sock , & event ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "add sock[%d] to wait EPOLLOUT event failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "add sock[%d] to wait EPOLLOUT event" , psession->sock );
		return 0;
	}
}

int ModifyInputSockFromEpoll( int epoll_socks , struct SocketSession *psession )
{
	struct epoll_event	event ;
	
	int			nret = 0 ;
	
	memset( & event , 0x00 , sizeof(event) );
	event.data.ptr = psession ;
	event.events = EPOLLIN | EPOLLERR ;
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_MOD , psession->sock , & event ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "modify sock[%d] to wait EPOLLIN event failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "modify sock[%d] to wait EPOLLIN event" , psession->sock );
		return 0;
	}
}

int ModifyOutputSockFromEpoll( int epoll_socks , struct SocketSession *psession )
{
	struct epoll_event	event ;
	
	int			nret = 0 ;
	
	memset( & event , 0x00 , sizeof(event) );
	event.data.ptr = psession ;
	event.events = EPOLLOUT | EPOLLERR ;
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_MOD , psession->sock , & event ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "modify sock[%d] to wait EPOLLOUT event failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "modify sock[%d] to wait EPOLLOUT event" , psession->sock );
		return 0;
	}
}

int DeleteSockFromEpoll( int epoll_socks , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "delete sock[%d] from epoll failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "delete sock[%d] from epoll" , psession->sock );
		close( psession->sock );
		return 0;
	}
}

int SetAddrReused( int sock )
{
	int	on ;
	
	on = 1 ;
	setsockopt( sock , SOL_SOCKET , SO_REUSEADDR , (void *) & on, sizeof(on) );
	DebugLog( __FILE__ , __LINE__ , "set sock[%d] addr reused" , sock );
	
	return 0;
}

int SetNonBlocking( int sock )
{
	int	opts;
	
	opts = fcntl( sock , F_GETFL ) ;
	if( opts < 0 )
	{
		return -1;
	}
	
	opts = opts | O_NONBLOCK;
	if( fcntl( sock , F_SETFL , opts ) < 0 )
	{
		return -2;
	}
	
	DebugLog( __FILE__ , __LINE__ , "set sock[%d] nonblock" , sock );
	
	return 0;
}

void SetSocketAddr( struct sockaddr_in *p_sockaddr , char *ip , long port )
{
	memset( p_sockaddr , 0x00 , sizeof(struct sockaddr_in) );
	p_sockaddr->sin_family = AF_INET ;
	p_sockaddr->sin_addr.s_addr = inet_addr( ip ) ;
	p_sockaddr->sin_port = htons( (unsigned short)port );
	return;
}

void GetSocketAddr( struct sockaddr_in *addr , char *ip , long *port )
{
	if( ip )
	{
#ifdef inet_ntop
		inet_ntop( AF_INET , addr->sin_addr , ip , MAXLEN_SERVER_IP + 1 );
#else
		strcpy( ip , inet_ntoa( addr->sin_addr ) );
#endif
	}
	
	if( port )
	{
		(*port) = (long)ntohs( addr->sin_port ) ;
	}
	
	return;
}

int BindListenSocket( char *ip , long port , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	psession->sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( psession->sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "socket failed[%d]errno[%d]" , psession->sock , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "socket ok , sock[%d]" , psession->sock );
	}
	
	SetAddrReused( psession->sock );
	
	strcpy( psession->ip , ip );
	psession->port = port ;
	SetSocketAddr( & (psession->addr) , ip , port );
	nret = bind( psession->sock , (struct sockaddr *) & (psession->addr) , sizeof(struct sockaddr) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "bind[%s:%d] failed[%d]errno[%d]" , ip , port , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "bind[%s:%d] ok" , ip , port );
	}
	
	nret = listen( psession->sock , MAXCNT_LISTEN_BAKLOG ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "listen[%s:%d] failed[%d]errno[%d]" , ip , port , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "listen[%s:%d] ok" , ip , port );
	}
	
	return 0;
}

int AsyncConnectSocket( char *ip , long port , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	psession->sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( psession->sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "socket failed[%d]errno[%d]" , psession->sock , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "socket ok , sock[%d]" , psession->sock );
	}
	
	SetNonBlocking( psession->sock );
	
	strcpy( psession->ip , ip );
	psession->port = port ;
	SetSocketAddr( & (psession->addr) , psession->ip , psession->port );
	nret = connect( psession->sock , (struct sockaddr *) & (psession->addr) , sizeof(struct sockaddr) ) ;
	if( nret )
	{
		if( errno == EINPROGRESS )
		{
			DebugLog( __FILE__ , __LINE__ , "connect2[%s:%d] ok" , psession->ip , psession->port );
			return RETURN_CONNECTING_IN_PROGRESS;
		}
		else
		{
			ErrorLog( __FILE__ , __LINE__ , "connect[%s:%d] failed[%d]errno[%d]" , psession->ip , psession->port , psession->sock , errno );
			close( psession->sock );
			return -1;
		}
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "connect[%s:%d] ok" , psession->ip , psession->port );
	}
	
	return 0;
}

int AcceptSocket( int epoll_socks , int listen_sock , struct SocketSession *psession )
{
	socklen_t		addr_len ;
	
	memset( & (psession->addr) , 0x00 , sizeof(struct sockaddr_in) );
	addr_len = sizeof(struct sockaddr_in) ;
	psession->sock = accept( listen_sock , (struct sockaddr *) & (psession->addr) , & addr_len ) ;
	if( psession->sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "[%d]accept failed , errno[%d]" , listen_sock , errno );
		return -1;
	}
	else
	{
		GetSocketAddr( & (psession->addr) , psession->ip , & (psession->port) );
		InfoLog( __FILE__ , __LINE__ , "[%d]accept[%d] ok , from[%s:%d]" , listen_sock , psession->sock , psession->ip , psession->port );
	}
	
	return 0;
}

int DiscardAcceptSocket( int epoll_socks , int listen_sock )
{
	struct SocketSession	session ;
	socklen_t		addr_len ;
	
	memset( & session , 0x00 , sizeof(struct SocketSession) );
	addr_len = sizeof(struct sockaddr_in) ;
	session.sock = accept( listen_sock , (struct sockaddr *) & (session.addr) , & addr_len ) ;
	if( session.sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "[%d]accept failed , errno[%d]" , listen_sock , errno );
		return -1;
	}
	else
	{
		GetSocketAddr( & (session.addr) , session.ip , & (session.port) );
		InfoLog( __FILE__ , __LINE__ , "[%d]accept[%d] ok , from[%s:%d]" , listen_sock , session.sock , session.ip , session.port );
	}
	
	close( session.sock );
	InfoLog( __FILE__ , __LINE__ , "discard accepted sock[%d]" , session.sock );
	
	return 0;
}

int AsyncReceiveSocketData( int epoll_socks , struct SocketSession *psession , int change_mode_flag )
{
	int		len ;
	
	int		nret = 0 ;
	
	if( psession->recv_body_len == 0 )
	{
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , LEN_COMMHEAD , 0 ) ;
	}
	else
	{
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , LEN_COMMHEAD + psession->recv_body_len - psession->total_recv_len , 0 ) ;
	}
	if( len < 0 )
	{
		if( _ERRNO == _EWOULDBLOCK )
			return RETURN_RECEIVING_IN_PROGRESS;
		
		ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on receiving data" , psession->sock );
		return -1;
	}
	else if( len == 0 )
	{
		if( psession->total_recv_len == 0 )
		{
			InfoLog( __FILE__ , __LINE__ , "detected sock[%d] closed on waiting for first byte" , psession->sock );
			return RETURN_PEER_CLOSED;
		}
		else
		{
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] closed on receiving data" , psession->sock );
			return -1;
		}
	}
	else
	{
		int	short_len ;
		
		short_len = len ;
		if( short_len > 4096 )
			short_len = 4096 ;
		DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer + psession->total_recv_len , short_len , "received sock[%d] [%d]bytes" , psession->sock , len );
		
		psession->total_recv_len += len ;
	}
	
	if( change_mode_flag == OPTION_ASYNC_CHANGE_MODE_FLAG && psession->comm_protocol_mode == 0 )
	{
		if( '0' <= psession->recv_buffer[0] && psession->recv_buffer[0] <= '9' )
		{
			psession->comm_protocol_mode = COMMPROTO_PRELEN ;
		}
		else
		{
			psession->comm_protocol_mode = COMMPROTO_LINE ;
			return RETURN_CHANGE_COMM_PROTOCOL_MODE;
			
		}
	}
	
	if( psession->recv_body_len == 0 )
	{
		if( psession->total_recv_len >= LEN_COMMHEAD )
		{
			psession->recv_body_len = ns2i( psession->recv_buffer , LEN_COMMHEAD ) ;
			DebugLog( __FILE__ , __LINE__ , "calc body len[%d]" , psession->recv_body_len );
			if( psession->recv_body_len <= 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "sock[%d] body len invalid[%d]" , psession->sock , psession->recv_body_len );
				return -1;
			}
			
			if( LEN_COMMHEAD + psession->recv_body_len > psession->recv_buffer_size-1 )
			{
				nret = ReallocRecvBuffer( psession , LEN_COMMHEAD + psession->recv_body_len + 1 ) ;
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "ReallocRecvBuffer ->[%d]bytes failed[%ld] , errno[%d]" , LEN_COMMHEAD + psession->recv_body_len + 1 , nret , errno );
					return -1;
				}
				else
				{
					DebugLog( __FILE__ , __LINE__ , "ReallocRecvBuffer ->[%d]bytes ok" , psession->recv_buffer_size );
				}
			}
		}
	}
	
	if( psession->recv_body_len > 0 )
	{
		if( psession->total_recv_len == LEN_COMMHEAD + psession->recv_body_len )
		{
			psession->bak = psession->recv_buffer[ LEN_COMMHEAD + psession->recv_body_len ] ;
			psession->recv_buffer[ LEN_COMMHEAD + psession->recv_body_len ] = '\0' ;
			return 0;
		}
	}
	
	return RETURN_RECEIVING_IN_PROGRESS;
}

int AfterDoProtocol( struct SocketSession *psession )
{
	if( psession->total_recv_len == LEN_COMMHEAD + psession->recv_body_len )
	{
		CleanRecvBuffer( psession );
		DebugLog( __FILE__ , __LINE__ , "clean recv buffer" );
	}
	else
	{
		memmove( psession->recv_buffer , psession->recv_buffer + LEN_COMMHEAD + psession->recv_body_len , psession->total_recv_len - LEN_COMMHEAD - psession->recv_body_len );
		psession->recv_buffer[0] = psession->bak ;
		psession->total_recv_len -= LEN_COMMHEAD + psession->recv_body_len ;
		psession->recv_body_len = 0 ;
		DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer , psession->total_recv_len , "recv buffer remain [%d]bytes" , psession->total_recv_len );
	}
	
	if( psession->total_send_len == 0 )
	{
		DebugLog( __FILE__ , __LINE__ , "proto return ok , but no data need sending" );
		return RETURN_NO_SEND_RESPONSE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "proto return ok , sock[%d] convert to wait EPOLLOUT event" , psession->sock );
		return 0;
	}
}

int AsyncReceiveCommand( int epoll_socks , struct SocketSession *psession , int skip_recv_flag )
{
	int		len ;
	
	if( ! ( skip_recv_flag == OPTION_ASYNC_SKIP_RECV_FLAG ) )
	{
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , psession->recv_buffer_size-1 - psession->total_recv_len , 0 ) ;
		if( len < 0 )
		{
			if( _ERRNO == _EWOULDBLOCK )
				return RETURN_RECEIVING_IN_PROGRESS;
			
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on receiving data" , psession->sock );
			return -1;
		}
		else if( len == 0 )
		{
			if( psession->total_recv_len == 0 )
			{
				InfoLog( __FILE__ , __LINE__ , "detected sock[%d] closed on waiting for first byte" , psession->sock );
				return RETURN_PEER_CLOSED;
			}
			else
			{
				ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] closed on receiving data" , psession->sock );
				return -1;
			}
		}
		else
		{
			int	short_len ;
			
			short_len = len ;
			if( short_len > 4096 )
				short_len = 4096 ;
			DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer + psession->total_recv_len , short_len , "received sock[%d] [%d]bytes" , psession->sock , len );
			
			psession->total_recv_len += len ;
		}
	}
	
	psession->newline = strchr( psession->recv_buffer , '\n' ) ;
	if( psession->newline == NULL )
		return RETURN_RECEIVING_IN_PROGRESS;
	
	*(psession->newline) = '\0' ;
	
	return 0;
}
	
int AfterDoCommandProtocol( struct SocketSession *psession )
{
	if( *(psession->newline+1) == '\0' )
	{
		CleanRecvBuffer( psession );
		DebugLog( __FILE__ , __LINE__ , "clean recv buffer" );
	}
	else
	{
		/* "command1\ncommand2\n" */
		int	len ;
		len = strlen(psession->newline+1) ;
		strcpy( psession->recv_buffer , psession->newline+1 );
		psession->total_recv_len = len ;
		DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer , psession->total_recv_len , "recv buffer remain [%d]bytes" , psession->total_recv_len );
	}
	
	if( psession->total_send_len == 0 )
	{
		DebugLog( __FILE__ , __LINE__ , "proto return ok , but no data need send" );
		return RETURN_NO_SEND_RESPONSE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "proto return ok , sock[%d] convert to wait EPOLLOUT event" , psession->sock );
		return 0;
	}
}

int AsyncSendSocketData( int epoll_socks , struct SocketSession *psession )
{
	int			len ;
	
	if( psession->total_send_len == 0 )
	{
		ModifyInputSockFromEpoll( epoll_socks , psession );
		return 0;
	}
	
	len = (int)send( psession->sock , psession->send_buffer + psession->send_len , psession->total_send_len - psession->send_len , 0 ) ;
	if( len < 0 )
	{
		if( _ERRNO == _EWOULDBLOCK )
			return 0;
		
		ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on sending data" , psession->sock );
		return -1;
	}
	else
	{
		int	short_len ;
		
		short_len = len ;
		if( short_len > 4096 )
			short_len = 4096 ;
		DebugHexLog( __FILE__ , __LINE__ , psession->send_buffer + psession->send_len , short_len , "sended sock[%d] [%d]bytes" , psession->sock , len );
		
		psession->send_len += len ;
	}
	
	return 0;
}

int SyncConnectSocket( char *ip , long port , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	psession->sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( psession->sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "socket failed[%d]errno[%d]" , psession->sock , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "socket ok , sock[%d]" , psession->sock );
	}
	
	strcpy( psession->ip , ip );
	psession->port = port ;
	SetSocketAddr( & (psession->addr) , psession->ip , psession->port );
	nret = connect( psession->sock , (struct sockaddr *) & (psession->addr) , sizeof(struct sockaddr) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "connect[%s:%d] failed[%d]errno[%d]" , psession->ip , psession->port , psession->sock , errno );
		close( psession->sock );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "connect[%s:%d] ok" , psession->ip , psession->port );
	}
	
	return 0;
}

int SyncReceiveSocketData( struct SocketSession *psession )
{
	long			len ;
	
	CleanRecvBuffer( psession );
	
	while( psession->total_recv_len < 8 )
	{
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , 8 - psession->total_recv_len , 0 ) ;
		if( len < 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on receiving data" , psession->sock );
			return -1;
		}
		else if( len == 0 )
		{
			if( psession->total_recv_len == 0 )
			{
				InfoLog( __FILE__ , __LINE__ , "detected sock[%d] close on waiting for first byte" , psession->sock );
				return RETURN_PEER_CLOSED;
			}
			else
			{
				ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] close on receiving data" , psession->sock );
				return -1;
			}
		}
		else
		{
			int	short_len ;
			
			short_len = len ;
			if( short_len > 4096 )
				short_len = 4096 ;
			DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer + psession->total_recv_len , short_len , "received sock[%d] [%d]bytes" , psession->sock , len );
			
			psession->total_recv_len += len ;
		}
	}
	
	psession->recv_body_len = ns2i( psession->recv_buffer , LEN_COMMHEAD ) ;
	
	while( psession->total_recv_len < 8 + psession->recv_body_len )
	{
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , 8 + psession->recv_body_len - psession->total_recv_len , 0 ) ;
		if( len < 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on receiving data" , psession->sock );
			return -1;
		}
		else if( len == 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] close on receiving data" , psession->sock );
			return -1;
		}
		else
		{
			int	short_len ;
			
			short_len = len ;
			if( short_len > 4096 )
				short_len = 4096 ;
			DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer + psession->total_recv_len , short_len , "received sock[%d] [%d]bytes" , psession->sock , len );
			
			psession->total_recv_len += len ;
		}
	}
	
	return 0;
}

int SyncSendSocketData( struct SocketSession *psession )
{
	int			len ;
	
	while( psession->send_len < psession->total_send_len )
	{
		len = (int)send( psession->sock , psession->send_buffer + psession->send_len , psession->total_send_len - psession->send_len , 0 ) ;
		if( len < 0 )
		{
			if( _ERRNO == _EWOULDBLOCK )
				return RETURN_SENDING_IN_PROGRESS;
			
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on sending data" , psession->sock );
			return -1;
		}
		else
		{
			int	short_len ;
			
			short_len = len ;
			if( short_len > 4096 )
				short_len = 4096 ;
			DebugHexLog( __FILE__ , __LINE__ , psession->send_buffer + psession->send_len , short_len , "sended sock[%d] [%d]bytes" , psession->sock , len );
			
			psession->send_len += len ;
		}
	}
	
	CleanSendBuffer( psession );
	
	return 0;
}

void FormatSendHead( struct SocketSession *psession , char *msg_type , int msg_len )
{
	char		pre_buffer[ LEN_COMMHEAD + LEN_MSGHEAD + 1 ] ;
	
	psession->total_send_len = LEN_COMMHEAD + LEN_MSGHEAD + msg_len ;
	psession->send_len = 0 ;
	memset( pre_buffer , 0x00 , sizeof(pre_buffer) );
	snprintf( pre_buffer , sizeof(pre_buffer) , "%0*d%-*s" , LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , LEN_MSGHEAD , msg_type );
	memcpy( psession->send_buffer , pre_buffer , LEN_COMMHEAD + LEN_MSGHEAD );
	
	return;
}

static int HexExpand( char *HexBuf , int HexBufLen , char *AscBuf )
{
	int     i,j=0;
	char    c;
	
	for(i=0;i<HexBufLen;i++){
		c=(HexBuf[i]>>4)&0x0f;
		if(c<=9) AscBuf[j++]=c+'0';
		else AscBuf[j++]=c+'A'-10;
		
		c=HexBuf[i]&0x0f;
		if(c<=9) AscBuf[j++]=c+'0';
		else AscBuf[j++]=c+'A'-10;
	}
	AscBuf[j]=0;
	return(0);
}

int FileMd5( char *pathfilename , char *md5_exp )
{
	MD5_CTX		context ;
	FILE		*fp = NULL ;
	char		buf[ 4096 + 1 ] ;
	int		size ;
	char		md5[ MD5_DIGEST_LENGTH + 1 ] ;
	
	MD5_Init( & context );
	
	fp = fopen( pathfilename , "rb" ) ;
	if( fp == NULL )
	{
		return -1;
	}
	
	while( ! feof(fp) )
	{
		memset( buf , 0x00 , sizeof(buf) );
		size = (int)fread( buf , sizeof(char) , sizeof(buf)-1 , fp ) ;
		if( size == 0 )
			break;
		
		MD5_Update( & context , buf , size );
	}
	
	fclose( fp );
	
	memset( md5 , 0x00 , sizeof(md5) );
	MD5_Final( (void*)md5 , & context );
	
	HexExpand( md5 , MD5_DIGEST_LENGTH , md5_exp );
	
	return 0;
}

