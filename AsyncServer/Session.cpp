#include "Session.h"

Session::Session(tcp::socket socket) : m_socket(std::move(socket)) { }

void Session::run()
{
	waitForRequest();
	checkDeadline();
}

void Session::waitForRequest()
{
	// �.�. ����� ��������� `this` ��� ��������� ������, ��� ����� ������� shared_from_this()
	auto self(shared_from_this());
	// ��� ������ �������� ������, ����� �������� ������
	http::async_read(m_socket, m_buffer, m_request,
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			// ���� ������ �� ����, ������ ��� ������ ������
			if (!ec) {
				self->processingRequest();
			} else {
				consoleCol(col::br_red);
				std::wcerr << L"������ ������: " << utf82wideUtf(ec.message()) << std::endl;
				consoleCol(col::cancel);
			}
		});
}

void Session::processingRequest()
{
	m_response.version(m_request.version());
	m_response.keep_alive(false);

	switch (m_request.method())
	{
	case http::verb::get:
		m_response.result(http::status::ok);
		m_response.set(http::field::server, "Beast");
		createResponseGet();
		break;
	
	case http::verb::post:
		m_response.result(http::status::ok);
		m_response.set(http::field::server, "Beast");
		createResponsePost();
		break;

	default:
		m_response.result(http::status::bad_request);
		m_response.set(http::field::content_type, "text/plain");
		beast::ostream(m_response.body())
			<< "Invalid request-method '"
			<< std::string(m_request.method_string())
			<< "'";
		break;
	}

	writeResponse();
}

void Session::createResponseGet()
{
	if (m_request.target() == "/")
	{
		m_response.set(http::field::content_type, "text/html");
		beast::ostream(m_response.body())
			<< "<html>\n"
			<< "<head><meta charset=\"UTF-8\"><title>Search Engine</title></head>\n"
			<< "<body>\n"
			<< "<h1>Search Engine</h1>\n"
			<< "<p>Welcome!<p>\n"
			<< "<form action=\"/\" method=\"post\">\n"
			<< "    <label for=\"search\">Search:</label><br>\n"
			<< "    <input type=\"text\" id=\"search\" name=\"search\"><br>\n"
			<< "    <input type=\"submit\" value=\"Search\">\n"
			<< "</form>\n"
			<< "</body>\n"
			<< "</html>\n";
	}
	else
	{
		m_response.result(http::status::not_found);
		m_response.set(http::field::content_type, "text/plain");
		beast::ostream(m_response.body()) << "File not found\r\n";
	}
}

void Session::createResponsePost()
{
	if (m_request.target() == "/")
	{
		std::string s = url_decode(buffers_to_string(m_request.body().data()));

		size_t pos = s.find('=');
		if (pos == std::string::npos)
		{
			m_response.result(http::status::not_found);
			m_response.set(http::field::content_type, "text/plain");
			beast::ostream(m_response.body()) << "File not found\r\n";
			return;
		}

		std::string key = s.substr(0, pos);
		if (key != "search")
		{
			m_response.result(http::status::not_found);
			m_response.set(http::field::content_type, "text/plain");
			beast::ostream(m_response.body()) << "File not found\r\n";
			return;
		}

		// ������� ������� �����
		std::vector<std::string> word;
		while (true)
		{
			s = s.substr(pos + 1);
			pos = s.find('+');
			if (pos != std::string::npos) {
				word.push_back(s.substr(0, pos));
			}
			else {
				word.push_back(s);
				break;
			}
		}

		std::wcout << L"���� �����:\n";
		for (const auto& w : word) {
			std::wcout << '\t' << w << '\n';
		}




		std::vector<std::string> searchResult = {
			"https://en.wikipedia.org/wiki/Main_Page",
			"https://en.wikipedia.org/wiki/Wikipedia",
		};






		m_response.set(http::field::content_type, "text/html");
		beast::ostream(m_response.body())
			<< "<html>\n"
			<< "<head><meta charset=\"UTF-8\"><title>Search Engine</title></head>\n"
			<< "<body>\n"
			<< "<h1>Search Engine</h1>\n"
			<< "<p>Response:<p>\n"
			<< "<ul>\n";

		for (const auto& url : searchResult) {

			beast::ostream(m_response.body())
				<< "<li><a href=\""
				<< url << "\">"
				<< url << "</a></li>";
		}

		beast::ostream(m_response.body())
			<< "</ul>\n"
			<< "</body>\n"
			<< "</html>\n";
	}
	else
	{
		m_response.result(http::status::not_found);
		m_response.set(http::field::content_type, "text/plain");
		beast::ostream(m_response.body()) << "File not found\r\n";
	}
}

std::string Session::url_decode(const std::string& encoded)
{
	std::string res;
	std::istringstream iss(encoded);
	char ch;

	while (iss.get(ch)) {
		if (ch == '%') {
			int hex;
			iss >> std::hex >> hex;
			res += static_cast<char>(hex);
		}
		else {
			res += ch;
		}
	}

	return res;
}

void Session::writeResponse()
{
	auto self(shared_from_this());

	m_response.content_length(m_response.body().size());

	http::async_write(m_socket, m_response,
		[self](beast::error_code ec, std::size_t)
		{
			self->m_socket.shutdown(tcp::socket::shutdown_send, ec);
			self->m_deadline.cancel();
		});
}

void Session::checkDeadline()
{
	auto self(shared_from_this());

	m_deadline.async_wait(
		[self](beast::error_code ec) {
			if (!ec) {
				self->m_socket.close(ec);
			}
		});
}
