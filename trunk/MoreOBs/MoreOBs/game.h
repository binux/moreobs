#ifndef GAME_H
#define GAME_H

//===============================game status==================================
#define STATUS_COMINGUPNEXT     0x00
#define STATUS_ABOUTTOSTART     0x01
#define STATUS_STARTED          0x02
#define STATUS_LIVE             0x03
#define STATUS_COMPLETED        0x04
#define STATUS_BROKEN           0x05
#define STATUS_INCOMPLETE       0x06

class CGame
{
private:
	string m_name;
	string m_players;
	string m_map;
	string m_streamer;
	uint32_t m_replayId;
	uint32_t m_originalId;
	uint32_t m_newId;
	uint32_t m_createTime;
	uint32_t m_startTime;
	BYTEARRAY m_version;
	BYTEARRAY m_options;
	BYTEARRAY m_mapOptions;
	uint32_t m_state;
	BYTEARRAY m_details;
	BYTEARRAY m_startHead;
	vector<BYTEARRAY> m_data;

public: 
	CGame ( string streamer , string gameName , uint32_t replayId , uint32_t gameId , string players , uint32_t createTime , BYTEARRAY version , BYTEARRAY options , string map , BYTEARRAY mapOptions );
	~CGame ( );
	string GetGameName ( )		{ return m_name; }
	string GetPlayers ( )		{ return m_players; }
	string GetMap ( )			{ return m_map; }
	uint32_t GetReplayId ( )	{ return m_replayId; }
	uint32_t GetId ( );
	uint32_t GetStartTime ( )	{ return m_startTime; }
	uint32_t GetState ( )		{ return m_state; }
	BYTEARRAY GetVersion ( )	{ return m_version; }
	BYTEARRAY GetOptions ( )	{ return m_options; }
	BYTEARRAY GetMapOptions ( )	{ return m_mapOptions; }
	BYTEARRAY GetDetails ( )	{ return m_details; }
	BYTEARRAY GetStartHead ( )	{ return m_startHead; }
	vector<BYTEARRAY>& GetGameData ( )	{ return m_data; }

	void SetDetials ( BYTEARRAY& b )	{ m_details.assign(b.begin() , b.end() ); }
	void SetStartHead ( BYTEARRAY& b )	{ m_startHead.assign( b.begin() , b.end() ); }
	void SetState ( uint32_t i )		{ m_state = i; }
	void SetStartTime ( uint32_t i )	{ m_startTime = i; }
	void SetGameData ( BYTEARRAY& b )	{ m_data.push_back(b); }
};

#endif