#ifndef CONTROL_H
#define CONTROL_H

#include <boost/asio.hpp>

#define Control_Waiting	    0x00
#define Control_Welcome	    0x01
#define Control_Username    0x02
#define Control_Password    0x03
#define Control_Login	    0x04

class CMoreObs;
class CConfig;

class CControl
{
private:
    uint32_t m_port;
    boost::asio::io_service* m_ioService;
    boost::asio::ip::tcp::acceptor* m_acceptor;
    boost::asio::ip::tcp::socket* m_socket;
    string b_receive;
    string b_send;
    unsigned char t_buffer[1024];

    CMoreObs* m_moreobs;
    uint32_t m_state;
    uint32_t m_lastAction;
    string m_username;
    CConfig* m_userdb;

    void handle_accept( const boost::system::error_code& error );
    void handle_read( const boost::system::error_code& error, std::size_t bytes_transferred );
    void DoSend( );
    void ExtractCommand( );
public:
    CControl( boost::asio::io_service* ioservice, uint32_t port, CMoreObs* moreobs );
    ~CControl( );
    void Run( );
    void Send ( string s );
    void Send ( BYTEARRAY b );
};

#endif
