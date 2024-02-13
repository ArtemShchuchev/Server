#pragma once

#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "Clientdb.h"
#include "SecondaryFunction.h"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

// �.�. Session ��� ������ ��� shared ptr, � ����� ��� �����������
// `this`, ������� ��� ����� ������������� �� enable_shared_from_this
class Session : public std::enable_shared_from_this<Session>
{
private:
    tcp::socket m_socket;
    net::streambuf m_buffer;
    http::request<http::dynamic_body> m_request;
    http::response<http::dynamic_body> m_response;
    net::steady_timer m_deadline{
        m_socket.get_executor(), std::chrono::seconds(60) };
    ConnectData m_cdata;

    void waitForRequest();
    void processingRequest();
    void createResponseGet();
    void createResponsePost();
    std::string url_decode(const std::string& encoded);
    void writeResponse();
    void checkDeadline();
	
public:
    // ����������� ������ ��������� socket
    Session(tcp::socket socket, const ConnectData& cdata);
    
    // run ��� ��� ������ �� ����� �������, ������ �� ������ ���� ��������
    void run();
};
