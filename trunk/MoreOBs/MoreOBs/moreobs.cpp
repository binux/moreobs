#include "includes.h"
#include "config.h"
#include "moreobs.h"
#include "client.h"
#include "protocol.h"

#include <boost/bind.hpp>

CMoreObs::CMoreObs ( CConfig *cfg )
{
	m_port = cfg->GetInt("listen_port",10383);
	protocol = new Protocol();
	serverName = cfg->GetString("server_name","MoreObs");
	about = cfg->GetString("server_about","A personal WTV server!");
	information = cfg->GetString("server_information","By Binux@CN"); 
	ircAdress = cfg->GetString("server_irc","none");
	webAdress = cfg->GetString("server_web","binux.yo2.cn");
	uploadRate = cfg->GetInt("server_uploadRate",0);

	try
	{
		CONSOLE_Print("[MOREOBS] start listening !",DEBUG_LEVEL_MESSAGE);
		m_ioService = new boost::asio::io_service;
		m_acceptor = new boost::asio::ip::tcp::acceptor(*m_ioService,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),m_port));
		CClient * t_socket = new CClient(m_ioService,this);
		m_acceptor->async_accept(*t_socket->GetSocket(),boost::bind(&CMoreObs::handle_accept,this,t_socket,boost::asio::placeholders::error));
	}
	catch(boost::system::system_error& e)
	{
		CONSOLE_Print(string("[MOREOBS]") + e.what() , DEBUG_LEVEL_ERROR);
	}
}


CMoreObs::~CMoreObs(void)
{
}

void CMoreObs::Run ( )
{
	m_ioService->run();
}

bool CMoreObs::Update( )
{
	//acceptor
	return true;
}

void CMoreObs::handle_accept( CClient * t_socket , const boost::system::error_code& error )
{
	if (!error)
	{
		try
		{
			t_socket->Run();
			DEBUG_Print("[MOREOBS] Get a new client !",DEBUG_LEVEL_PACKET);
			CClient * t_socket = new CClient(m_ioService,this);
			m_acceptor->async_accept(*t_socket->GetSocket(),boost::bind(&CMoreObs::handle_accept,this,t_socket,boost::asio::placeholders::error));
		}
		catch(boost::system::system_error& e)
		{
			CONSOLE_Print(string("[MOREOBS]") + e.what() , DEBUG_LEVEL_ERROR);
		}
	}
}
