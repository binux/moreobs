#ifndef CONTROL_H
#define CONTROL_H

#include <boost/asio.hpp>

class CMoreObs;

class CControl
{
private:
    CMoreObs* m_moreobs;
    uint32_t m_port;
    boost::asio::io_service* m_ioService;
    boost::asio::ip::tcp::acceptor* m_acceptor;
    boost::asio::ip::tcp::socket* m_socket;
    string b_receive;
    string b_send;
    char t_buffer[1024];

    void handle_accept( const boost::system::error_code& error );
    void handle_read( const boost::system::error_code& error, std::size_t bytes_transferred );
public:
    CControl( boost::asio::io_service* ioservice, uint32_t port, CMoreObs* moreobs );
    ~CControl( );
    void Run( );
};

#endif
