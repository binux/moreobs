#include "includes.h"
#include "util.h"
#include "client.h"
#include "moreobs.h"
#include "protocol.h"
#include "game.h"
#include "gamelist.h"

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

CClient::CClient(boost::asio::io_service* io_service , CMoreObs* moreobs)
{
    m_ioService = io_service;
    m_moreobs = moreobs;

    m_socket = new boost::asio::ip::tcp::socket(*io_service);
    t_buffer = new unsigned char[1024];

    m_protocol = m_moreobs->protocol;
    m_game = NULL;
    m_state = CLIENT_STATE_NOT_READY;
    m_type = CLIENT_TYPE_UNDEFINE;
    m_lastRecv = GetTime();
    m_lastSend = GetTime();
    m_lastUpdateTime = 0;
    m_gameDataPacketCount = 0;
    m_gameDataPos = 0;

}

CClient::~CClient ( )
{
    if(t_buffer)
    {
        delete t_buffer;
    }
    
    if(m_socket)
    {
        m_socket->close( );
        delete m_socket;
    }

    //if( m_state == CLIENT_STATE_SUBSCRIBED && m_game )
    //{
    //    m_game->RemoveClient( this );
    //}
}

void CClient::Run ( )
{
    CONSOLE_Print("[CLIENT] Get a new client from [" + m_socket->remote_endpoint( ).address( ).to_string( ) + "]",DEBUG_LEVEL_MESSAGE);
    m_socket->async_read_some(boost::asio::buffer(t_buffer,1024),boost::bind(&CClient::handle_read,this,t_buffer,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
    m_state = CLIENT_STATE_HAND_SHAKING;
}

void CClient::Update ( ) 
{
    if( m_game )
    {
	//subscribed game update
	if(( m_state == CLIENT_STATE_SUBSCRIBED || 
	     m_state == CLIENT_STATE_GAMEDETAIL || 
	     m_state == CLIENT_STATE_GAMEDSTART || 
	     m_state == CLIENT_STATE_GAMEDATA ) 
	&& ( m_lastUpdateTime < m_game->GetLastUpdateTime( ) ))
	{
	    //m_lastUpdateTime = GetTime ( );

	    switch( m_state )
	    {
	    case CLIENT_STATE_SUBSCRIBED:
		if( m_game->GetState( ) >= STATUS_ABOUTTOSTART)
		{
		    Send( m_protocol->SEND_GAMEDETAIL( m_game ) );
		    m_state = CLIENT_STATE_GAMEDETAIL;
		}
	    case CLIENT_STATE_GAMEDETAIL:
		if( m_game->GetState( ) >= STATUS_STARTED )
		{
		    Send( m_protocol->SEND_GAMEDSTART( m_game ) );
		    m_state = CLIENT_STATE_GAMEDATA;
		}
	    case CLIENT_STATE_GAMEDSTART:
	    case CLIENT_STATE_GAMEDATA:
		if( m_game->GetState( ) >= STATUS_LIVE )
		{
		    BYTEARRAY t_buffer = m_protocol->SEND_GAMEDATA( m_game, m_gameDataPacketCount, m_gameDataPos );
		    if(!t_buffer.empty( ))
			Send( t_buffer );
		    else if( m_game->GetState( ) == STATUS_COMPLETED )
		    {
			Send( m_protocol->SEND_FINISHED( m_game ) );
		    }
		}
		break;
	    default :
		break;
	    }
	}

	if( m_game->GetDelete( ) )
	{
            Send( m_protocol->SEND_UNSUBSCRIBGAME( m_game->GetId( ) ) );
	    m_state = CLIENT_STATE_LOGIN;
	    m_game = NULL;
	}
    }

    DoSend ( );
}

void CClient::Close ( )
{
    if(m_socket)
    {
        m_socket->close( );
    }

    if(m_game)
    {
	m_game = NULL;
    }

    m_state = CLIENT_STATE_DELETE_READY;
}

void CClient::handle_read ( unsigned char * t_buffer , const boost::system::error_code& error , std::size_t bytes_transferred )
{
    if(!error)
    {
        try
        {
            DEBUG_Print(UTIL_CreateByteArray(t_buffer,bytes_transferred),DEBUG_LEVEL_PACKET_RECV);
            m_lastRecv = GetTime();
            b_receive.insert( b_receive.size(), (char *)t_buffer, bytes_transferred);
            m_socket->async_read_some(boost::asio::buffer(t_buffer,1024),boost::bind(&CClient::handle_read,this,t_buffer,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        }
        catch(boost::system::system_error& e)
        {
            CONSOLE_Print(string("[CLIENT]") + e.what() , DEBUG_LEVEL_ERROR);
        }
    }
    else
    {
        CONSOLE_Print("[CLIENT] Connection closed cleanly by peer!",DEBUG_LEVEL_PACKET_RECV);
        Close( );
        return ;
    }

    ExtractPacket();
}

void CClient::Send ( string s )
{
    b_send += s;
}

void CClient::Send( BYTEARRAY b )
{
    //DEBUG_Print(b,4);
    b_send.append(b.begin(),b.end());
}

void CClient::ExtractPacket( )
{
    BYTEARRAY t_bytes;

    //hand shacking
    if(m_state == CLIENT_STATE_HAND_SHAKING && b_receive.size() >= 4)
    {
        t_bytes = UTIL_CreateByteArray( (unsigned char *)b_receive.c_str( ), 4 );

        if(UTIL_ByteArrayToUInt32( t_bytes, false ) == (uint32_t)LOGIN_PROTOCOL )
        {
            Send( UTIL_CreateByteArray ( (uint32_t)LOGIN_PROTOCOL,false ) );
            b_receive = b_receive.substr( 4 );
            m_state = CLIENT_STATE_NEGOTIATING;
        }
	else
	{
	    Close( );
	}
    }

    //packet
    while(b_receive.size() >= 5 )
    {
        t_bytes = UTIL_CreateByteArray( (unsigned char *)b_receive.c_str( ), 5 );

        if(t_bytes[0] == HEADER_1 && t_bytes[1] == HEADER_2 )
        {
            uint16_t len = UTIL_ByteArrayToUInt16(t_bytes,false,3);
            if( len > b_receive.size() )
                break;

            BYTEARRAY t_packet = UTIL_CreateByteArray( (unsigned char *)b_receive.c_str( ) + 5, len - 5 );
            //DEBUG_Print( BYTEARRAY( t_bytes.begin( ) , t_bytes.begin( ) + len ), DEBUG_LEVEL_PACKET);
            switch(t_bytes[2])
            {
                case PACKET_TYPE_NEGOTIATING     :
                    m_type = m_protocol->RECV_NEGOTIATION(t_packet);
                    if(m_type)
                    {
                        Send(m_protocol->SEND_NEGOTIATION( m_type ));
                        m_state = CLIENT_STATE_AUTHORIZING;
                    }
                    break;
                case PACKET_TYPE_AUTHORIZING     :
                    if(m_protocol->RECV_AUTHORIZING(t_packet,&m_clientVersion,&m_clientProtocolVersion,&m_clientName,&m_username,&m_password))
                    {
                        Send(m_protocol->SEND_AUTHORIZING(m_moreobs->serverName,m_moreobs->about,m_moreobs->information,m_moreobs->ircAdress,m_moreobs->webAdress));
                        m_state = CLIENT_STATE_LOGIN;
                    }
                    break;
                /*----------------------only recorder will use this---------------------------*/
                case PACKET_TYPE_SERVERDETAIL    :
                    Send(m_protocol->SEND_SERVERDETAIL(m_moreobs->uploadRate));
                    break;
                case PACKET_TYPE_CREATEREQUEST   :
                    if(m_type == CLIENT_TYPE_RECORDER && m_state == CLIENT_STATE_LOGIN )
                    {
                        m_game = m_protocol->RECV_CREATEREQUEST(t_packet , m_username );
                        m_game->SetGameId( m_moreobs->gameList->New(m_game) );
                        if(m_game)
                        {
                            CONSOLE_Print("[CLIENT] created a new game :" + m_game->GetGameName( ) , DEBUG_LEVEL_MESSAGE);
                            Send(m_protocol->SEND_CREATEREQUEST( m_game ));
                            m_state = CLIENT_STATE_UPDATA;
                        }
                    }
                    break;
                case PACKET_TYPE_GAMEDETAILU     :
                    if(m_type == CLIENT_TYPE_RECORDER && m_state == CLIENT_STATE_UPDATA )
                    {
                        if(m_protocol->RECV_GAMEDETAILU(t_packet,m_game))
                            ;
                    }
                    break;
                case PACKET_TYPE_GAMESTARTU      :
                    if(m_type == CLIENT_TYPE_RECORDER && m_state == CLIENT_STATE_UPDATA )
                    {
                        if(m_protocol->RECV_GAMESTARTU(t_packet,m_game))
                            ;
                    }
                    break;
                case PACKET_TYPE_GAMEDATAU       :
                    if(m_type == CLIENT_TYPE_RECORDER && m_state == CLIENT_STATE_UPDATA )
                    {
                        if(m_protocol->RECV_GAMEDATAU(t_packet,m_game))
                            ;
                    }
                    break;
                case PACKET_TYPE_GAMEEND         :
                    if(m_type == CLIENT_TYPE_RECORDER && m_state == CLIENT_STATE_UPDATA )
                    {
                        if(m_protocol->RECV_GAMEEND(t_packet,m_game))
                        {
                            m_state = CLIENT_STATE_LOGIN;
                            m_game = NULL;
                        }
                    }
                    break;
                case PACKET_TYPE_FINISHEDU       :
                    if(m_type == CLIENT_TYPE_RECORDER && m_state == CLIENT_STATE_UPDATA )
                    {
                        if(m_protocol->RECV_FINISHEDU(t_packet,m_game))
                        {
                            Send(m_protocol->SEND_FINISHEDU(m_game));
                            m_state = CLIENT_STATE_LOGIN;
                            m_game = NULL;
                        }
                    }
                    break;
                /*-----------------------only client will use this-----------------------------*/
                case PACKET_TYPE_GETLIST         :
                    //m_protocol->RECV_GETLIST(t_packet);
                    Send(m_protocol->SEND_GAMELIST( m_moreobs->gameList ));
                    break;
                case PACKET_TYPE_DETAIL          :
                    Send( m_protocol->SEND_DETAIL( m_moreobs->gameList->FindGameById( m_protocol->RECV_GETDETAIL( t_packet ) ) ) );
                    break;
                case PACKET_TYPE_SUBSCRIBGAME    :
                {
                    uint32_t t_gameId = m_protocol->RECV_SUBSCRIBGAME( t_packet );
                    m_game = m_moreobs->gameList->FindGameById( t_gameId );
                    if( m_game )
                    {
                        m_state = CLIENT_STATE_SUBSCRIBED;
                        Send( m_protocol->SEND_SUBSCRIBGAME( m_game->GetId( ) ));
                    }
                    else
                        Send( m_protocol->SEND_UNSUBSCRIBGAME( t_gameId ) );
                }
                    break;
                case PACKET_TYPE_UNSUBSCRIBGAME  :
                {
                    uint32_t t_gameId = m_protocol->RECV_UNSUBSCRIBGAME( t_packet );
                    Send( m_protocol->SEND_UNSUBSCRIBGAME( m_game->GetId( ) ) );
                    m_game = NULL;
                    m_state = CLIENT_STATE_LOGIN;
                }
                    break;
                case PACKET_TYPE_GAMEDETAIL      :
                    break;
                case PACKET_TYPE_GAMEDSTART      :
                    break;
                case PACKET_TYPE_GAMEDATA        :
                    m_protocol->RECV_GAMEDATA( t_packet, m_gameDataPacketCount, m_gameDataPos );
                    m_gameDataPacketCount++;
                    Update( );
                    break;
                case PACKET_TYPE_FINISHED        :
                    break;
                default :
                    CONSOLE_Print("[CLIENT] unknow packet type : " + UTIL_ToString(t_bytes[2]) ,DEBUG_LEVEL_ERROR );
                    break;
            }
            try
            {
                b_receive = b_receive.substr( len );
            }
            catch(boost::system::system_error& e)
            {
                CONSOLE_Print(string("[CLIENT]") + e.what() , DEBUG_LEVEL_ERROR);
            }
        }
        else
        {
            b_receive.clear();
        }
    }

    DoSend ( );
}

void CClient::DoSend ( )
{
    if( m_state == CLIENT_STATE_NOT_READY )
	return ;

    while(!b_send.empty())
    {
	m_lastSend = GetTime();
        b_send = b_send.substr( m_socket->send( boost::asio::buffer(b_send) ) );
    }
}
