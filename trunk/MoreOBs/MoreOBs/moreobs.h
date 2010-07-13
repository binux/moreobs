#ifndef MOREOBS_H
#define MOREOBS_H

#include <boost\asio.hpp>

class CConfig;
class CClient;
class Protocol;
class CGame;
class CGameList;

class CMoreObs
{
private:
	uint32_t m_port;
	boost::asio::io_service *m_ioService;
	boost::asio::ip::tcp::acceptor *m_acceptor;
public:
	Protocol* protocol;
	string serverName;
	string about;
	string information; 
	string ircAdress;
	string webAdress;
	uint32_t uploadRate;
	CGameList* gameList;

	CMoreObs( CConfig *cfg );
	~CMoreObs(void);
	void Run ( );
	bool Update( );
	void handle_accept(  CClient * t_socket , const boost::system::error_code& error );
};

#endif