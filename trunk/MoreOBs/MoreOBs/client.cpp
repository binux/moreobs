#include "includes.h"
#include "util.h"
#include "client.h"
#include "moreobs.h"
#include "protocol.h"
#include "game.h"
#include "gamelist.h"

#include <boost/bind.hpp>

CClient::CClient(boost::asio::io_service* io_service , CMoreObs* moreobs)
{
	m_ioService = io_service;
	m_socket = new boost::asio::ip::tcp::socket(*io_service);
	t_buffer = NULL;

	m_moreobs = moreobs;
	m_protocol = m_moreobs->protocol;
	m_game = NULL;
	m_state = CLIENT_STATE_HAND_SHAKING;
	m_type = CLIENT_TYPE_UNDEFINE;
	m_lastRecv = m_lastSend = GetTime();

}

CClient::~CClient ( )
{
	if(t_buffer)
	{
		delete t_buffer;
		t_buffer = NULL;

		m_socket->close();
		delete m_socket;
	}
}

void CClient::Run ( )
{
	if(!t_buffer)
	{
		t_buffer = new unsigned char[1024];
		m_socket->async_read_some(boost::asio::buffer(t_buffer,1024),boost::bind(&CClient::handle_read,this,t_buffer,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
	}
}

void CClient::handle_read ( unsigned char * t_buffer , const boost::system::error_code& error , std::size_t bytes_transferred )
{
	if(!error)
	{
		try
		{
			//DEBUG_Print(UTIL_CreateByteArray(t_buffer,bytes_transferred),DEBUG_LEVEL_PACKET);
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
		DEBUG_Print("[CLIENT] Connection closed cleanly by peer!",DEBUG_LEVEL_PACKET);
		delete this;
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
	b_send.append(b.begin(),b.end());
}

void CClient::ExtractPacket( )
{
	BYTEARRAY t_bytes = UTIL_CreateByteArray( (unsigned char *)b_receive.c_str( ), b_receive.size( ) );

	//hand shacking
	if(m_state == CLIENT_STATE_HAND_SHAKING)
	{
		if(UTIL_ByteArrayToUInt32( t_bytes, false ) == (uint32_t)LOGIN_PROTOCOL )
		{
			Send( UTIL_CreateByteArray ( (uint32_t)LOGIN_PROTOCOL,false ) );

			b_receive = b_receive.substr( 4 );
			t_bytes = UTIL_SubByteArray( t_bytes, 4 );

			m_state = CLIENT_STATE_NEGOTIATING;
		}
	}

	while(t_bytes.size() >= 5 )
	{
		if(t_bytes[0] == HEADER_1 && t_bytes[1] == HEADER_2 )
		{
			uint16_t len = UTIL_ByteArrayToUInt16(t_bytes,false,3);
			if( len > t_bytes.size() )
				break;

			BYTEARRAY t_packet = UTIL_SubByteArray( t_bytes, 5, len - 5 );
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
						m_game->SetGameId( m_moreobs->gameList->NewGame(m_game) );
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
					Send( m_protocol->SEND_GAMEDETAIL( m_moreobs->gameList->FindGameById( m_protocol->RECV_GETDETAIL( t_packet ) ) ) );
					break;
				case PACKET_TYPE_SUBSCRIBGAME    :
					break;
				case PACKET_TYPE_UNSUBSCRIBGAME  :
					break;
				case PACKET_TYPE_GAMEDETAIL      :
					break;
				case PACKET_TYPE_GAMEDSTART      :
					break;
				case PACKET_TYPE_GAMEDATA        :
					break;
				case PACKET_TYPE_FINISHED        :
					break;
				default :
					DEBUG_Print("[CLIENT] unknow packet type : " + UTIL_ToString(t_bytes[2]) ,DEBUG_LEVEL_ERROR );
					break;
			}
			try
			{
				b_receive = b_receive.substr( len );
				t_bytes = UTIL_SubByteArray( t_bytes, len );
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
	m_lastSend = GetTime();
	while(!b_send.empty())
		b_send = b_send.substr( m_socket->send( boost::asio::buffer(b_send) ) );
}