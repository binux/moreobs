#include "includes.h"
#include "util.h"
#include "control.h"
#include "moreobs.h"
#include "gamelist.h"
#include "game.h"

#include <boost/bind.hpp>

CControl::CControl( boost::asio::io_service* ioservice, uint32_t port, CMoreObs* moreobs )
{
    m_ioService = ioservice;
    m_port = port;
    m_moreobs = moreobs;
    m_acceptor = new boost::asio::ip::tcp::acceptor(*m_ioService,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),m_port));
    m_socket = new boost::asio::ip::tcp::socket(*m_ioService);
}

CControl::~CControl( )
{
    if(m_acceptor)
	delete m_acceptor;
    if(m_socket)
	delete m_socket;
}

void CControl::Run( )
{
    m_acceptor->async_accept(*m_socket,boost::bind(&CControl::handle_accept,this,boost::asio::placeholders::error));
}

void CControl::handle_read ( const boost::system::error_code& error , std::size_t bytes_transferred )
{
    if(!error)
    {
        try
        {
            DEBUG_Print( string( t_buffer, bytes_transferred ), DEBUG_LEVEL_PACKET_RECV);
            b_receive.insert( b_receive.size(), (char *)t_buffer, bytes_transferred);
            m_socket->async_read_some(boost::asio::buffer(t_buffer,1024),boost::bind(&CControl::handle_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        }
        catch(boost::system::system_error& e)
        {
            CONSOLE_Print(string("[CONTROL]") + e.what() , DEBUG_LEVEL_ERROR);
        }
    }
    else
    {
        CONSOLE_Print("[CONTROL] Connection closed cleanly by peer!",DEBUG_LEVEL_PACKET_RECV);
	m_acceptor->async_accept(*m_socket,boost::bind(&CControl::handle_accept,this,boost::asio::placeholders::error));
        return ;
    }
}

void CControl::handle_accept( const boost::system::error_code& error )
{
    if (!error)
    {
        try
        {
	    m_socket->async_read_some(boost::asio::buffer(t_buffer,1024),boost::bind(&CControl::handle_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        }
        catch(boost::system::system_error& e)
        {
            CONSOLE_Print(string("[MOREOBS]") + e.what() , DEBUG_LEVEL_ERROR);
        }
    }
}
