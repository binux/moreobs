#ifndef MOREOBS_H
#define MOREOBS_H

#include <boost/asio.hpp>

class CConfig;
class CClient;
class CClientList;
class Protocol;
class CGame;
class CGameList;
class CControl;

class CMoreObs
{
private:
	uint32_t m_port;
	uint32_t m_controlPort;
	boost::asio::io_service *m_ioService;
	boost::asio::ip::tcp::acceptor *m_acceptor;
	boost::asio::deadline_timer* m_timer;
public:
	Protocol* protocol;
	string serverName;
	string about;
	string information; 
	string ircAdress;
	string webAdress;
	uint32_t uploadRate;
	uint32_t updateTimer;
	CGameList* gameList;
	CClientList* clientList;
	CControl* control;
	bool shutdown;

	CMoreObs( CConfig *cfg );
	~CMoreObs(void);
	void Run ( );
	bool Update( );
	void handle_accept(  CClient * t_socket , const boost::system::error_code& error );
	void handle_timer( const boost::system::error_code& error );
};

#endif
