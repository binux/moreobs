#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>

class CMoreObs;
class Protocol;

//==============================client type===================================
#define CLIENT_TYPE_UNDEFINE    0x00
#define CLIENT_TYPE_CLIENT      0x01
#define CLIENT_TYPE_RECORDER    0x02
//=============================client state===================================
#define CLIENT_STATE_NOT_READY	     0xf0
#define CLIENT_STATE_HAND_SHAKING    0xf1
#define CLIENT_STATE_DELETE_READY    0xf2
#define CLIENT_STATE_NEGOTIATING     0x00
#define CLIENT_STATE_AUTHORIZING     0x01
#define CLIENT_STATE_LOGIN           0x20
#define CLIENT_STATE_UPDATA          0x11
#define CLIENT_STATE_SUBSCRIBED      0x22
#define CLIENT_STATE_GAMEDETAIL      0x24
#define CLIENT_STATE_GAMEDSTART      0x25
#define CLIENT_STATE_GAMEDATA        0x26

class CGame;

class CClient
{
private:
    boost::asio::ip::tcp::socket* m_socket;
    boost::asio::io_service* m_ioService;
    unsigned char * t_buffer;
    string b_receive;
    string b_send;

    CMoreObs* m_moreobs;
    Protocol* m_protocol;
    CGame* m_game;
    uint32_t m_state;
    uint32_t m_type;
    uint32_t m_lastRecv;
    uint32_t m_lastSend;
    uint32_t m_lastUpdateTime;
    uint32_t m_gameDataPacketCount;
    uint32_t m_gameDataPos;
    uint32_t m_clientVersion;
    uint32_t m_clientProtocolVersion;
    string m_clientName;
    string m_username;
    string m_password;
    
public:
    CClient(boost::asio::io_service* io_service , CMoreObs* moreobs);
    ~CClient();
    void Run ( );
    void Update ( );
    void Close ( );
    CGame* GetGame( )		{ return m_game; }
    uint32_t GetState( )	{ return m_state; }
    uint32_t GetDeleteReady( )	{ return m_state == CLIENT_STATE_DELETE_READY; }

    boost::asio::ip::tcp::socket* GetSocket( ) { return m_socket; }
    string GetUserName( ) { return m_username; }

private:
    void handle_read ( unsigned char * t_buffer , const boost::system::error_code& error , std::size_t bytes_transferred );
    void Send ( string s );
    void Send ( BYTEARRAY b );
    void ExtractPacket ( );
    void DoSend ( );
};

#endif
