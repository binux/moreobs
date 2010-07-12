#ifndef CLIENT_H
#define CLIENT_H

#include <boost\asio.hpp>

class CMoreObs;
class Protocol;

//==============================client type===================================
#define CLIENT_TYPE_UNDEFINE    0x00
#define CLIENT_TYPE_CLIENT      0x01
#define CLIENT_TYPE_RECORDER    0x02
//=============================client state===================================
#define CLIENT_STATE_HAND_SHAKING	0xff
#define CLIENT_STATE_NEGOTIATING	0x00
#define CLIENT_STATE_AUTHORIZING	0x01
#define CLIENT_STATE_LOGIN			0x20
#define CLIENT_STATE_UPDATA			0x11

class CGame;

class CClient
{
private:
	boost::asio::ip::tcp::socket * m_socket;
	boost::asio::io_service * m_ioService;
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
	uint32_t m_clientVersion;
	uint32_t m_clientProtocolVersion;
	string m_clientName;
	string m_username;
	string m_password;
public:
	CClient(boost::asio::io_service* io_service , CMoreObs* moreobs);
	~CClient();
	void Run ( );
	void handle_read ( unsigned char * t_buffer , const boost::system::error_code& error , std::size_t bytes_transferred );
	void Send ( string s );
	void Send ( BYTEARRAY b );
	void ExtractPacket ( );
	void DoSend ( );

	boost::asio::ip::tcp::socket* GetSocket( ) { return m_socket; }
	string GetUserName( ) { return m_username; }
};

#endif