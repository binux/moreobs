#include "includes.h"
#include "client.h"
#include "game.h"
#include "util.h"

CGame::CGame ( string streamer , string gameName , uint32_t replayId , uint32_t gameId , string players , uint32_t createTime , BYTEARRAY version , BYTEARRAY options , string map , BYTEARRAY mapOptions )
{
	m_name = gameName;
	m_players = players;
	m_map = map;
	m_streamer = streamer ;
	m_replayId = replayId;
	m_originalId = gameId;
	m_newId = 0;	//todotodo
	m_createTime = createTime;
	m_startTime = 0;
	m_lastUpdateTime = GetTime( );
	m_version = version;
	m_options = options;
	m_mapOptions = mapOptions;
	m_state = STATUS_COMINGUPNEXT;
	m_dataLenth = 0;
}

uint32_t CGame::GetId ( )
{
	if(m_newId)
		return m_newId;
	else
		return m_originalId;
}

void CGame::SetDetials ( BYTEARRAY& b )	
{
	m_details.assign( b.begin() , b.end() ); 
	m_state = STATUS_ABOUTTOSTART;
	m_lastUpdateTime = GetTime( ); 
}

void CGame::SetStartHead ( BYTEARRAY& b )	
{ 
	m_startHead.assign( b.begin() , b.end() ); 
	m_state = STATUS_STARTED;
	m_lastUpdateTime = GetTime( ); 
}

void CGame::SetState ( uint32_t i )		
{ 
	m_state = i; 
	m_lastUpdateTime = GetTime( ); 
}

void CGame::SetStartTime ( uint32_t i )	
{ 
	m_startTime = i; 
	m_state = STATUS_STARTED;
	m_lastUpdateTime = GetTime( ); 
}

void CGame::SetLastedTime ( uint32_t i )	
{ 
	m_lastedTime = i; 
	m_lastUpdateTime = GetTime( ); 
}

void CGame::NewGameData ( BYTEARRAY b )	
{ 
	m_data.insert(make_pair(m_dataLenth, b));
	m_lastedTime =  UTIL_ByteArrayToUInt32( b, false ) ;
	m_dataLenth += b.size( );
	m_state = STATUS_LIVE;
	m_lastUpdateTime = GetTime( ); 
}

void CGame::SetGameId ( uint32_t i )		
{ 
	m_newId = i; 
	m_lastUpdateTime = GetTime( ); 
}

void CGame::NewClient ( CClient* client ) 
{ 
	m_clients.push_back( client ); 
}

void CGame::RemoveClient ( CClient* client ) 
{ 
	m_clients.remove( client );
}