#include "includes.h"
#include "protocol.h"
#include "util.h"
#include "game.h"

BYTEARRAY Protocol::SEND_HAND_SHAKING ( )
{
	BYTEARRAY result;

	result.push_back(0x00);
	result.push_back(0x03);
	result.push_back(0x00);
	result.push_back(0x01);

	return result;
}

BYTEARRAY Protocol::SEND_NEGOTIATION ( uint32_t client_type )
{
	BYTEARRAY result;

	result.push_back(HEADER_1);
	result.push_back(HEADER_2);						//header
	result.push_back(PACKET_TYPE_NEGOTIATING);		//type
	result.push_back(0x00);
	result.push_back(0x00);							//len
	if( client_type == 0x01 )
		UTIL_AppendByteArray(result,(uint32_t)DOWNLOAD_PRTOCOL,false);
	else if( client_type == 0x02 )
		UTIL_AppendByteArray(result,(uint32_t)UPLOAD_PROTOCOL,false);
	else
		DEBUG_Print("[PROTOCOL] unknow client type : " + UTIL_ToString(client_type) , DEBUG_LEVEL_WARN );

	MakeLenth(result);
	return result;
}

BYTEARRAY Protocol::SEND_AUTHORIZING ( string serverName , string about , string information , string ircAdress , string webAdress)
{
	BYTEARRAY result;

	result.push_back(HEADER_1);
	result.push_back(HEADER_2);
	result.push_back(PACKET_TYPE_AUTHORIZING);
	result.push_back(0x00);
	result.push_back(0x00);
	result.push_back(0x01);		//unkonw
	UTIL_AppendByteArray(result,serverName);
	UTIL_AppendByteArray(result,(uint32_t)0x02000001,false);	//server version
	UTIL_AppendByteArray(result,about);
	UTIL_AppendByteArray(result,information);
	UTIL_AppendByteArray(result,ircAdress);
	UTIL_AppendByteArray(result,webAdress);

	MakeLenth(result);
	return result;
}

BYTEARRAY Protocol::SEND_SERVERDETAIL ( uint32_t maxUploadRate )
{
	BYTEARRAY result;
	
	result.push_back(HEADER_1);
	result.push_back(HEADER_2);
	result.push_back(PACKET_TYPE_SERVERDETAIL);
	result.push_back(0x00);
	result.push_back(0x00);
	UTIL_AppendByteArray(result,maxUploadRate,false);
	UTIL_AppendByteArray(result,GetTime(),false);
	
	MakeLenth(result);
	return result;
}

BYTEARRAY Protocol::SEND_CREATEREQUEST ( CGame* game )
{
	BYTEARRAY result;
	
	result.push_back(HEADER_1);
	result.push_back(HEADER_2);
	result.push_back(PACKET_TYPE_CREATEREQUEST);
	result.push_back(0x00);
	result.push_back(0x00);
	UTIL_AppendByteArray(result,game->GetReplayId(),false);
	UTIL_AppendByteArray(result,game->GetId(),false);
	result.insert(result.end(),16,0x00);

	MakeLenth(result);
	return result;
}


//========================================================================================================
uint32_t Protocol::RECV_NEGOTIATION ( BYTEARRAY b )
{
	if(b.size() < 5)
		return 0x00;

	return (uint32_t)b[0];
}

bool Protocol::RECV_AUTHORIZING ( BYTEARRAY b , uint32_t* clientVersion , uint32_t* protocolVersion , string* clientName , string* username ,string* password )
{
	if(b.size() < 12 )
		return false;

	uint32_t t_pos = 0;
	if(clientName)
		*clientName = UTIL_ToString(b,t_pos);
	t_pos += clientName->size() + 1;
	if(clientVersion)
		*clientVersion = UTIL_ByteArrayToUInt32(b,false, t_pos );
	t_pos += 4;
	if(protocolVersion)
		*protocolVersion = UTIL_ByteArrayToUInt32(b,false, t_pos );
	t_pos += 4;
	if(username)
		*username = UTIL_ToString(b,t_pos);
	t_pos += username->size() + 1;
	if(password)
		*password = UTIL_ToString(b,t_pos);

	return true;
}

CGame* Protocol::RECV_CREATEREQUEST ( BYTEARRAY b , string streamer )
{
	if(b.size() < 20 )
		return false;

	uint32_t t_pos = 0;
	uint32_t t_replayId = UTIL_ByteArrayToUInt32(b,false,t_pos ); t_pos += 4;
	uint32_t t_gameId  = UTIL_ByteArrayToUInt32(b,false,t_pos ); t_pos += 4;
	string t_gameName = UTIL_ToString(b,t_pos); t_pos += t_gameName.size() + 1;
	string t_players = UTIL_ToString(b,t_pos); t_pos += t_players.size() + 1;
	uint32_t t_startTime =  UTIL_ByteArrayToUInt32(b,false,t_pos ); t_pos += 4;
	BYTEARRAY t_version = UTIL_SubByteArray( b, t_pos, t_pos+8-1);t_pos += 12;
	BYTEARRAY t_options = UTIL_SubByteArray( b, t_pos, t_pos+20-1);t_pos += 20;
	string t_map = UTIL_ToString(b,t_pos); t_pos += t_map.size() + 1;
	BYTEARRAY t_mapOptions = UTIL_SubByteArray( b, t_pos, t_pos+8-1);

	CGame* t_game = new CGame(streamer,t_gameName,t_replayId,t_gameId,t_players,t_startTime,t_version,t_options,t_map,t_mapOptions);

	return t_game;
}

bool Protocol::RECV_GAMEDETAILU ( BYTEARRAY b , CGame* game )
{
	if(game->GetId() == UTIL_ByteArrayToUInt32(b,false) )
	{
		if(game->GetState() == STATUS_COMINGUPNEXT)
		{
			game->SetDetials( UTIL_SubByteArray( b, 4, b.size( ) ) );
			game->SetState(STATUS_ABOUTTOSTART);
			return true;
		}
		else
		{
			game->SetState(STATUS_BROKEN);
			return false;
		}
	}
	return false;
}


bool Protocol::RECV_GAMESTARTU ( BYTEARRAY b , CGame* game )
{
	if(game->GetId() == UTIL_ByteArrayToUInt32(b,false) )
	{
		if(game->GetState() == STATUS_ABOUTTOSTART)
		{
			game->SetStartHead( UTIL_SubByteArray( b, 0, 8 ) );
			game->SetStartTime( UTIL_ByteArrayToUInt32(b,false,9) );
			game->SetState( STATUS_STARTED );

			return true;
		}
		else
		{
			game->SetState(STATUS_BROKEN);
			return false;
		}
	}
	return false;
}


bool Protocol::RECV_GAMEDATAU ( BYTEARRAY b , CGame* game )
{
	if( b.size() < 8 )
		return false;

	if(game->GetId() == UTIL_ByteArrayToUInt32(b,false) )
	{
		if(game->GetState() == STATUS_STARTED || game->GetState() == STATUS_LIVE)
		{
			game->SetGameData( UTIL_SubByteArray( b, 8, b.size() ) );
			game->SetState(STATUS_LIVE);

			return true;
		}
		else
		{
			game->SetState(STATUS_BROKEN);
			return false;
		}
	}
	return false;
}

bool Protocol::RECV_GAMEEND ( BYTEARRAY b , CGame* game )
{
	if(game->GetId() == UTIL_ByteArrayToUInt32(b,false) )
	{
		if(game->GetState() == STATUS_STARTED || game->GetState() == STATUS_LIVE)
			{
			game->SetState(STATUS_INCOMPLETE);

			return true;
		}
		else
		{
			game->SetState(STATUS_BROKEN);
			return false;
		}
	}
	return false;
}

bool Protocol::RECV_FINISHEDU ( BYTEARRAY b , CGame* game )
{
	if(game->GetId() == UTIL_ByteArrayToUInt32(b,false) )
	{
		if(game->GetState() == STATUS_LIVE)
		{
			game->SetState(STATUS_COMPLETED);
			return true;
		}
		else
		{
			game->SetState(STATUS_BROKEN);
			return false;
		}
	}
	return false;
}

void Protocol::MakeLenth ( BYTEARRAY& b)
{
	uint16_t len = b.size();
	BYTEARRAY t_b = UTIL_CreateByteArray((uint16_t)len,false);
	b[3]=t_b[0];b[4]=t_b[1];

	return;
}