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

class CClient;

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
    uint32_t m_lastedTime;
    uint32_t m_lastUpdateTime;
    BYTEARRAY m_version;
    BYTEARRAY m_options;
    BYTEARRAY m_mapOptions;
    uint32_t m_state;
    BYTEARRAY m_details;
    BYTEARRAY m_startHead;
    map<uint32_t, BYTEARRAY> m_data;
    uint32_t m_dataLenth;
    //list<CClient*> m_clients;
    uint32_t m_clientCount;
    bool m_delete;
    bool m_deleteReady;

public: 
    CGame ( string streamer , string gameName , uint32_t replayId , uint32_t gameId , string players , uint32_t createTime , BYTEARRAY version , BYTEARRAY options , string map , BYTEARRAY mapOptions );
    //~CGame ( );
    string GetGameName ( )		{ return m_name; }
    string GetPlayers ( )		{ return m_players; }
    string GetStreamer ( )		{ return m_streamer; }
    string GetMap ( )			{ return m_map; }
    uint32_t GetReplayId ( )	{ return m_replayId; }
    uint32_t GetId ( );
    uint32_t GetStartTime ( )	{ return m_startTime; }
    uint32_t GetLastedTime ( )	{ return m_lastedTime; }
    uint32_t GetLastUpdateTime ( )	{ return m_lastUpdateTime; }
    uint32_t GetState ( )		{ return m_state; }
    BYTEARRAY& GetVersion ( )	{ return m_version; }
    BYTEARRAY& GetOptions ( )	{ return m_options; }
    BYTEARRAY& GetMapOptions ( )	{ return m_mapOptions; }
    BYTEARRAY& GetDetails ( )	{ return m_details; }
    BYTEARRAY& GetStartHead ( )	{ return m_startHead; }
    map<uint32_t, BYTEARRAY>& GetGameData ( )	{ return m_data; }
    bool GetDelete ( )		{ return m_delete; }
    bool GetDeleteReady ( )	{ return m_deleteReady; }

    void SetDetials ( BYTEARRAY b );
    void SetStartHead ( BYTEARRAY b );
    void SetState ( uint32_t i );
    void SetStartTime ( uint32_t i );
    void SetLastedTime ( uint32_t i );
    void NewGameData ( BYTEARRAY b );
    void SetGameId ( uint32_t i );
    void SetDelete ( )	{ m_delete = true; }

    //void NewClient ( CClient* client );
    //void RemoveClient ( CClient* client );

    void Update ( );
};

#endif
