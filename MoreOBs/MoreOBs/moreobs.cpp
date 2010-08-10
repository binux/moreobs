#include "includes.h"
#include "config.h"
#include "moreobs.h"
#include "client.h"
#include "protocol.h"
#include "game.h"
#include "gamelist.h"
#include "clientlist.h"
#include "control.h"

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

CMoreObs::CMoreObs ( CConfig *cfg )
{
    m_port = cfg->GetInt("listen_port",10383);
    m_controlPort = cfg->GetInt("control_port",10230);
    protocol = new Protocol( );
    serverName = cfg->GetString("server_name","MoreObs");
    about = cfg->GetString("server_about","A personal WTV server!");
    information = cfg->GetString("server_information","By Binux@CN"); 
    ircAdress = cfg->GetString("server_irc","none");
    webAdress = cfg->GetString("server_web","binux.yo2.cn");
    uploadRate = cfg->GetInt("server_uploadRate",1000000);
    updateTimer = cfg->GetInt("update_frequency",1000);
    gameList = new CGameList( );
    clientList = new CClientList( );
    shutdown = false;

    m_ioService = new boost::asio::io_service;
    m_acceptor = new boost::asio::ip::tcp::acceptor(*m_ioService,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),m_port));
    m_timer = new boost::asio::deadline_timer(*m_ioService, boost::posix_time::milliseconds(500));
    control = new CControl( m_ioService, m_controlPort, this );
}


CMoreObs::~CMoreObs(void)
{
    delete protocol;
    delete gameList;
    delete clientList;
    delete control;
    delete m_acceptor;
    delete m_timer;
    delete m_ioService;
}

void CMoreObs::Run ( )
{
    try
    {
        CONSOLE_Print("[MOREOBS] start listening !",DEBUG_LEVEL_MESSAGE);
        CClient* t_socket = new CClient(m_ioService,this);
	clientList->New(t_socket);
        m_acceptor->async_accept(*t_socket->GetSocket(),boost::bind(&CMoreObs::handle_accept,this,t_socket,boost::asio::placeholders::error));
	m_timer->expires_from_now( boost::posix_time::milliseconds( updateTimer ) );
	m_timer->async_wait(boost::bind(&CMoreObs::handle_timer,this,boost::asio::placeholders::error));
    }
    catch(boost::system::system_error& e)
    {
        CONSOLE_Print(string("[MOREOBS]") + e.what() , DEBUG_LEVEL_ERROR);
    }

    control->Run();
    m_ioService->run();
}

bool CMoreObs::Update( )
{
    if(shutdown)
    {
	m_ioService->stop( );
	return false;
    }

    clientList->Update( );
    gameList->Update( );
    return true;
}

void CMoreObs::handle_accept( CClient * t_socket , const boost::system::error_code& error )
{
    if (!error)
    {
        try
        {
            t_socket->Run();
            //CONSOLE_Print("[MOREOBS] Get a new client from [" + t_socket->GetSocket( )->remote_endpoint( ).address( ).to_string( ) + "]",DEBUG_LEVEL_MESSAGE);
            CClient* t_socket = new CClient(m_ioService,this);
	    clientList->New(t_socket);
            m_acceptor->async_accept(*t_socket->GetSocket(),boost::bind(&CMoreObs::handle_accept,this,t_socket,boost::asio::placeholders::error));
        }
        catch(boost::system::system_error& e)
        {
            CONSOLE_Print(string("[MOREOBS]") + e.what() , DEBUG_LEVEL_ERROR);
        }
    }
}

void CMoreObs::handle_timer( const boost::system::error_code& error )
{
    Update( );
    
    m_timer->expires_from_now( boost::posix_time::milliseconds( updateTimer ) );
    m_timer->async_wait(boost::bind(&CMoreObs::handle_timer,this,boost::asio::placeholders::error));
}
