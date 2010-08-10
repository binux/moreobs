#include "includes.h"
#include "util.h"
#include "control.h"
#include "moreobs.h"
#include "gamelist.h"
#include "game.h"
#include "clientlist.h"
#include "client.h"
#include "config.h"

#include <boost/bind.hpp>

CControl::CControl( boost::asio::io_service* ioservice, uint32_t port, CMoreObs* moreobs )
{
    m_ioService = ioservice;
    m_port = port;
    m_acceptor = new boost::asio::ip::tcp::acceptor(*m_ioService,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),m_port));
    m_socket = new boost::asio::ip::tcp::socket(*m_ioService);

    m_moreobs = moreobs;
    m_state = Control_Waiting;
    m_userdb = new CConfig();
    m_userdb->Read( "control_user.cfg" );
}

CControl::~CControl( )
{
    if(m_acceptor)
	delete m_acceptor;
    if(m_socket)
	delete m_socket;
    delete m_userdb;
}

void CControl::Run( )
{
    if(m_socket->is_open( ))
	m_socket->close( );
    m_state = Control_Waiting;
    b_receive.clear( );
    m_acceptor->async_accept(*m_socket,boost::bind(&CControl::handle_accept,this,boost::asio::placeholders::error));
}

void CControl::handle_read ( const boost::system::error_code& error , std::size_t bytes_transferred )
{
    if(!error)
    {
        try
        {
            DEBUG_Print(UTIL_CreateByteArray(t_buffer,bytes_transferred),DEBUG_LEVEL_PACKET_RECV);
            //DEBUG_Print( string( t_buffer, bytes_transferred ), DEBUG_LEVEL_PACKET_RECV);
            b_receive.insert( b_receive.size(), (char *)t_buffer, bytes_transferred);
            m_socket->async_read_some(boost::asio::buffer(t_buffer,1024),boost::bind(&CControl::handle_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        }
        catch(boost::system::system_error& e)
        {
            CONSOLE_Print(string("[CONTROL]") + e.what() , DEBUG_LEVEL_ERROR);
        }

	ExtractCommand( );
    }
    else
    {
        CONSOLE_Print("[CONTROL] Connection closed cleanly by peer!",DEBUG_LEVEL_PACKET_RECV);
	Run( );
        return ;
    }
}

void CControl::handle_accept( const boost::system::error_code& error )
{
    if (!error)
    {
        try
        {
            CONSOLE_Print("[CONTROL] Get a new client from [" + m_socket->remote_endpoint( ).address( ).to_string( ) + "]",DEBUG_LEVEL_MESSAGE);
	    m_socket->async_read_some(boost::asio::buffer(t_buffer,1024),boost::bind(&CControl::handle_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        }
        catch(boost::system::system_error& e)
        {
            CONSOLE_Print(string("[CONTROL]") + e.what() , DEBUG_LEVEL_ERROR);
        }

	ExtractCommand( );
    }
}

void CControl::Send ( string s )
{
    b_send += s;
    DoSend( );
}

void CControl::Send( BYTEARRAY b )
{
    b_send.append(b.begin(),b.end());
    DoSend( );
}

void CControl::DoSend ( )
{
    if( !m_socket )
	return ;

    while(!b_send.empty())
    {
        b_send = b_send.substr( m_socket->send( boost::asio::buffer(b_send) ) );
    }
}

size_t StringFindControlCharacter( string& s, size_t start = 0 )
{
    size_t i;
    for( i = start; i<s.size( ); i++ )
    {
	if(s[i]<32 || s[i]>127)
	    return i;
    }

    return i;
}

size_t StringFindNotControlCharacter( string& s, size_t start = 0 )
{
    size_t i;
    for( i = start; i<s.size( ); i++ )
    {
	if( 32<=s[i] && s[i]<=127 )
	    return i;
    }

    return i;
}

void CControl::ExtractCommand( )
{
    if( m_state == Control_Waiting )
    {
	//sending welcome message
	Send( "\nMoreOBs Control v0.03(r12)\n\n" );
	Send( "Username:" );
	m_state = Control_Username;
    }
    if( b_receive.empty( ) || StringFindControlCharacter( b_receive ) == b_receive.size( ) )	//check if command ends
	return ;

    string :: size_type CommandStart = b_receive.find_first_not_of( " " );
    string :: size_type CommandEnd = b_receive.find( " ", CommandStart );
    if( CommandEnd == string::npos )	//with out payload
	CommandEnd = StringFindControlCharacter( b_receive );
    string command = b_receive.substr( CommandStart, CommandEnd - CommandStart );

    string :: size_type PayloadStart = b_receive.find_first_not_of( " ", CommandEnd + 1 );
    string :: size_type PayloadEnd = StringFindControlCharacter( b_receive, PayloadStart );
    if( PayloadStart >= b_receive.size( ) )	//with out payload
	PayloadStart = PayloadEnd = b_receive.size( ) - 1;
    string payload = b_receive.substr( PayloadStart, PayloadEnd - PayloadStart );

    if( m_state == Control_Login )
    {
	CONSOLE_Print( "[CONTROL] Command-[" + command +"] Payload-[" + payload + "]", DEBUG_LEVEL_MESSAGE );

	//Commands
	//
	//  help - show help message
	if( command == "help" )
	    ;
	//  quit/exit - quit control 
	else if( command == "quit" || command == "exit" )
	{
	    Run( );
	    return ;
	}
	//  games - list all games
	else if( command == "games" )
	{
	    Send( "id\tname\n" );
	    foreach( CGame* game, m_moreobs->gameList->GetList( ) )
	    {
		Send( UTIL_ToString( game->GetId( ) ) + "\t" + game->GetGameName( ) + "\n" );
	    }
	}
	//  state - show state of server
	else if( command == "state" )
	{
	    Send( "MoreOBs is hosting " + UTIL_ToString( m_moreobs->gameList->GetSize( ) ) + " games and serving for " + UTIL_ToString( m_moreobs->clientList->GetSize( ) ) + " users.\n" );
	}
	//  shutdown
	else if( command == "shutdown" )
	{
	    Send( "MoreOBs is shutdown by user\n" );
	    m_moreobs->shutdown = true;
	}
	//  gamedel
	else if( command == "gamedel" )
	{
	    if(!payload.empty( ))
	    {
		CGame* game = m_moreobs->gameList->FindGameById( UTIL_ToUInt32( payload ) );
		if(game != NULL )
		    game->SetDelete( );
	    }
	}
	//  gameinfo
	else if( command == "gameinfo" )
	{
	    if(!payload.empty( ))
	    {
    
		CGame* game = m_moreobs->gameList->FindGameById( UTIL_ToUInt32( payload ) );
		if(game != NULL )
		{
		    Send( "gameid:\t" + UTIL_ToString( game->GetId( ) ) + "\n" );
		    Send( "gamename:\t" + game->GetGameName( ) + "\n" );
		    Send( "map:\t" + game->GetMap( ) + "\n" );
		    Send( "streamer:\t" + game->GetStreamer( ) + "\n" );
		    Send( "players:\t" + game->GetPlayers( ) + "\n" );
		}
	    }
	}
	
	Send( "MoreOBs> " );
    }
    else if( m_state == Control_Username )
    {
	CONSOLE_Print( "[CONTOL] User-[" + command + "] is trying to login! Waiting password.", DEBUG_LEVEL_MESSAGE );
	m_username = command;
	Send( "Password:" );
	m_state = Control_Password;
    }
    else if( m_state == Control_Password )
    {
	if( m_userdb->Exists( m_username ) && m_userdb->GetString( m_username, "" ) == command )
	    CONSOLE_Print( "[CONTOL] User-[" + command + "] login!", DEBUG_LEVEL_MESSAGE );
	else
	{
	    Run( );
	    return ;
	}

	Send( "MoreOBs> " );
	m_state = Control_Login;
    }

    b_receive = b_receive.substr( StringFindNotControlCharacter( b_receive, PayloadEnd ) );
    DoSend( );
}

