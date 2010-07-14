#ifndef PROTOCOL_H
#define PROTOCOL_H

#define HEADER_1 0xef
#define HEADER_2 0xbe

//===============================game status==================================
#define STATUS_COMINGUPNEXT     0x00
#define STATUS_ABOUTTOSTART     0x01
#define STATUS_STARTED          0x02
#define STATUS_LIVE             0x03
#define STATUS_COMPLETED        0x04
#define STATUS_BROKEN           0x05
#define STATUS_INCOMPLETE       0x06

//=================================packet type================================
/*---------------------both client and recorder will use this-----------------*/
#define PACKET_TYPE_NEGOTIATING     0x00	//negotiation
#define PACKET_TYPE_AUTHORIZING     0x01	//authorizing
/*----------------------only recorder will use this---------------------------*/
#define PACKET_TYPE_SERVERDETAIL    0x10	//sending request for serverDetail
#define PACKET_TYPE_CREATEREQUEST   0x11	//sending CreateRequest
#define PACKET_TYPE_GAMEDETAILU     0x12	//sending GameDetails(Update)
#define PACKET_TYPE_GAMESTARTU      0x13	//sending GameStart
#define PACKET_TYPE_GAMEDATAU       0x14	//send GameDate
#define PACKET_TYPE_GAMEEND         0x15    //Upload stoped by recorder
//unused?                           0x16
#define PACKET_TYPE_FINISHEDU       0x17    //Update finished
/*-----------------------only client will use this-----------------------------*/
#define PACKET_TYPE_GETLIST         0x20	//refresh gameList
#define PACKET_TYPE_DETAIL          0x21	//detail for all games
#define PACKET_TYPE_SUBSCRIBGAME    0x22	//subscribing game details
#define PACKET_TYPE_UNSUBSCRIBGAME  0x23	//Unsubscribing a game
#define PACKET_TYPE_GAMEDETAIL      0x24	//awaiting gamedetails
#define PACKET_TYPE_GAMEDSTART      0x25	//GameStart
#define PACKET_TYPE_GAMEDATA        0x26    //game date
#define PACKET_TYPE_FINISHED        0x27	//download finished
//unknow?                           0x28    //FIXME:has been seen with out knowing the usage

//==============================protocols=====================================
#define LOGIN_PROTOCOL      0x01000300
#define DOWNLOAD_PRTOCOL    0x01000500
#define UPLOAD_PROTOCOL     0x01000601

class CGame;
class CGameList;
class CClient;

class Protocol
{
public :
	//SEND
	BYTEARRAY SEND_HAND_SHAKING		( );
	BYTEARRAY SEND_NEGOTIATION		( uint32_t client_type );
	BYTEARRAY SEND_AUTHORIZING		( string serverName , string about , string information ,string ircAdress , string webAdress);
	BYTEARRAY SEND_SERVERDETAIL		( uint32_t maxUploadRate );
	BYTEARRAY SEND_CREATEREQUEST	( CGame* game );
	BYTEARRAY SEND_FINISHEDU		( CGame* game );
	BYTEARRAY SEND_GAMELIST			( CGameList* gameList );
	BYTEARRAY SEND_DETAIL			( CGame* game );
	BYTEARRAY SEND_SUBSCRIBGAME		( uint32_t gameId );
	BYTEARRAY SEND_UNSUBSCRIBGAME	( uint32_t gameId );
	BYTEARRAY SEND_GAMEDETAIL		( CGame* game );
	BYTEARRAY SEND_GAMEDSTART		( CGame* game );
	BYTEARRAY SEND_GAMEDATA			( CGame* game, uint32_t packetNum, uint32_t start );
	BYTEARRAY SEND_FINISHED			( CGame* game );

	//RECV
	uint32_t RECV_NEGOTIATION		( BYTEARRAY& b );
	bool RECV_AUTHORIZING			( BYTEARRAY& b, uint32_t* clientVersion, uint32_t* protocolVersion, string* clientName, string* username, string* password );
	CGame* RECV_CREATEREQUEST		( BYTEARRAY& b, string streamer );
	bool RECV_GAMEDETAILU			( BYTEARRAY& b, CGame* game );
	bool RECV_GAMESTARTU			( BYTEARRAY& b, CGame* game );
	bool RECV_GAMEDATAU				( BYTEARRAY& b, CGame* game );
	bool RECV_GAMEEND				( BYTEARRAY& b, CGame* game );
	bool RECV_FINISHEDU				( BYTEARRAY& b, CGame* game );
	uint32_t RECV_GETLIST			( BYTEARRAY& b );
	uint32_t RECV_GETDETAIL			( BYTEARRAY& b );
	uint32_t RECV_SUBSCRIBGAME		( BYTEARRAY& b );
	uint32_t RECV_UNSUBSCRIBGAME	( BYTEARRAY& b )	{ return RECV_SUBSCRIBGAME(b); }
	bool RECV_GAMEDATA				( BYTEARRAY& b, uint32_t& packet, uint32_t& pos );

private :
	void MakeLenth ( BYTEARRAY& b);
};
#endif