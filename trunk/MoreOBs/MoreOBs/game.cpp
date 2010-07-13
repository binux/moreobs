#include "includes.h"
#include "client.h"
#include "game.h"

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
	m_version = version;
	m_options = options;
	m_mapOptions = mapOptions;
	m_state = STATUS_COMINGUPNEXT;
}

uint32_t CGame::GetId ( )
{
	if(m_newId)
		return m_newId;
	else
		return m_originalId;
}