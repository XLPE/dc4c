/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

int		g_server_exit_flag = 0 ;

static void server_signal_proc( int signum )
{
	if( signum == SIGTERM )
		g_server_exit_flag = 1 ;
	return;
}

int server( struct ServerEnv *penv )
{
	time_t			now ;
	int			all_total_send_len ;
	long			epoll_timeout ;
	struct epoll_event	events[ WAIT_EVENTS_COUNT ] ;
	int			epoll_ready_count ;
	int			epoll_ready_index ;
	struct epoll_event	*pevent ;
	struct SocketSession	*psession = NULL ;
	int			rserver_index ;
	
	int			nret = 0 ;
	
	signal( SIGTERM , & server_signal_proc );
	
	SetLogFile( "%s/log/dc4c_wserver_%d_%s:%d.log" , getenv("HOME") , penv->wserver_index+1 , penv->param.wserver_ip , penv->param.wserver_port );
	if( penv->param.loglevel_debug == 1 )
		SetLogLevel( LOGLEVEL_DEBUG );
	else
		SetLogLevel( LOGLEVEL_INFO );
	
	penv->epoll_socks = epoll_create( EPOLL_FDS_COUNT ) ;
	if( penv->epoll_socks < 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "epoll_create failed[%d]errno[%d]" , penv->epoll_socks , errno );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "epoll_create ok , sock[%d]" , penv->epoll_socks );
	}
	
	nret = InitSocketSession( & (penv->listen_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		return -1;
	}
	
	nret = BindListenSocket( penv->param.wserver_ip , penv->param.wserver_port , & (penv->listen_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] failed[%d]errno[%d]" , penv->param.wserver_ip , penv->param.wserver_port , nret , errno );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] ok" , penv->param.wserver_ip , penv->param.wserver_port );
	}
	
	AddInputSockToEpoll( penv->epoll_socks , & (penv->listen_session) );
	
	nret = InitSocketSession( & (penv->accepted_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		return -1;
	}
	
	for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
	{
		nret = InitSocketSession( & (penv->connect_session[rserver_index]) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
			return -1;
		}
		
		nret = comm_AsyncConnectToRegisterServer( penv , & (penv->connect_session[rserver_index]) , 0 ) ;
		if( nret == RETURN_CONNECTING_IN_PROGRESS )
		{
			InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer done" );
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer failed[%d]" , nret );
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer ok" );
		}
	}
	
	all_total_send_len = 0 ;
	epoll_timeout = 1 ;
	while( g_server_exit_flag == 0 || ( IsSocketEstablished( & (penv->accepted_session) ) && penv->accepted_session.total_send_len > 0 ) )
	{
		if( getppid() == 1 && g_server_exit_flag == 0 )
		{
			g_server_exit_flag = 1 ;
		}
		
		memset( events , 0x00 , sizeof(events) );
		epoll_ready_count = epoll_wait( penv->epoll_socks , events , WAIT_EVENTS_COUNT , epoll_timeout * 1000 ) ;
		
		for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
		{
			if( IsSocketClosed( & (penv->connect_session[rserver_index]) ) )
			{
				nret = comm_AsyncConnectToRegisterServer( penv , & (penv->connect_session[rserver_index]) , 0 ) ;
				if( nret == RETURN_CONNECTING_IN_PROGRESS )
				{
					InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer done" );
				}
				else if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer failed[%d]" , nret );
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer ok" );
				}
			}
		}
		
		DebugLog( __FILE__ , __LINE__ , "epoll_wait [%d]events reached" , epoll_ready_count );
		
		time( & now );
		
		for( epoll_ready_index = 0 , pevent = & (events[0]) ; epoll_ready_index < epoll_ready_count ; epoll_ready_index++ , pevent++ )
		{
			psession = pevent->data.ptr ;
			DebugLog( __FILE__ , __LINE__ , "psession[%p] pevent->events[%d]" , psession , pevent->events );
			
			if( psession == & (penv->listen_session) )
			{
				if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on listen sock[%d]" , psession->sock );
					g_server_exit_flag = 1 ;
					break;
				}
				else if( pevent->events & EPOLLIN || pevent->events & EPOLLHUP )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on listen sock[%d]" , psession->sock );
					
					nret = comm_OnListenSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
					break;
				}
				else if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on listen sock[%d]" , psession->sock );
					g_server_exit_flag = 1 ;
					break;
				}
			}
			else if( & (penv->connect_session[0]) <= psession && psession <= & (penv->connect_session[RSERVER_ARRAYSIZE-1]) )
			{
				if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketError( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLIN || pevent->events & EPOLLHUP )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketOutput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
			}
			else if( psession == & (penv->executing_session) )
			{
				if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on info pipe[%d]" , psession->sock );
					g_server_exit_flag = 1 ;
					break;
				}
				else if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on info pipe[%d]" , psession->sock );
					
					nret = comm_OnInfoPipeError( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLIN || pevent->events & EPOLLHUP )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on info pipe[%d]" , psession->sock );
					
					nret = comm_OnInfoPipeInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
			}
			else if( psession == & (penv->accepted_session) )
			{
				if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketError( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLIN || pevent->events & EPOLLHUP )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketOutput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
			}
			else
			{
				if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on wild sock[%d]" , psession->sock );
					
					nret = comm_OnWildSocketError( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on wild sock[%d]" , psession->sock );
					
					nret = comm_OnWildSocketOutput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
			}
		}
		
		app_HeartBeatRequest( penv , & now , & epoll_timeout );
	}
	
	for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
	{
		if( IsSocketEstablished( & (penv->connect_session[rserver_index]) ) )
		{
			DeleteSockFromEpoll( penv->epoll_socks , & (penv->connect_session[rserver_index]) );
			CloseSocket( & (penv->connect_session[rserver_index]) );
			CleanSocketSession( & (penv->connect_session[rserver_index]) );
		}
	}
	
	if( IsSocketEstablished( & (penv->accepted_session) ) )
	{
		DeleteSockFromEpoll( penv->epoll_socks , & (penv->accepted_session) );
		CloseSocket( & (penv->accepted_session) );
		CleanSocketSession( & (penv->accepted_session) );
	}
	
	if( IsSocketEstablished( & (penv->listen_session) ) )
	{
		DeleteSockFromEpoll( penv->epoll_socks , & (penv->listen_session) );
		CloseSocket( & (penv->listen_session) );
		CleanSocketSession( & (penv->listen_session) );
	}
	
	close( penv->epoll_socks );
	InfoLog( __FILE__ , __LINE__ , "close all socks and epoll_socks" );
	
	InfoLog( __FILE__ , __LINE__ , "--- wserver [%s:%d] - [%d] --- end" , penv->param.wserver_ip , penv->param.wserver_port , penv->wserver_index+1 );
	
	return 0;
}
