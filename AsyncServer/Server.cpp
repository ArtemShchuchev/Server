#include "Server.h"

Server::Server(net::io_context& io_context, unsigned port)
: m_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
	// вызываем do_accept(), где ждем подключения клиентов
	do_accept();
}

void Server::do_accept()
 {
	// это асинхронный прием, который означает,
	// что лямбда-функция выполняется при подключении клиента.
	m_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
		if (!ec) {
			// создаем сессию, и сразу же вызываем функцию запуска.
			// Примечание: здесь сокет передается в лямбду
			std::make_shared<Session>(std::move(socket))->run();
		} else {
			consoleCol(col::br_red);
			std::wcerr << L"Ошибка сервера: " << utf82wideUtf(ec.message()) << std::endl;
			consoleCol(col::cancel);
		}
		// поскольку мы хотим, чтобы несколько клиентов подключились,
		// дождитесь следующего, вызвав do_accept()
		do_accept();
	});
}
