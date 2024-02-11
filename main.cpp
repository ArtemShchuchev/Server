#include <iostream>
#include "Clientdb.h"
#include "ConfigFile.h"
#include "Session.h"
#include "Server.h"
#include "SecondaryFunction.h"

int main(int argc, char* argv[])
{
	setRuLocale();

	try
	{
        ConfigFile config("../config.ini");
        const unsigned serverPort(config.getConfig<unsigned>("httpServer", "port"));
        ConnectData connectDb;
        connectDb.dbname = config.getConfig<std::string>("BdConnect", "dbname");
        connectDb.host = config.getConfig<std::string>("BdConnect", "host");
        connectDb.password = config.getConfig<std::string>("BdConnect", "password");
        connectDb.username = config.getConfig<std::string>("BdConnect", "user");
        connectDb.port = config.getConfig<unsigned>("BdConnect", "port");
        ////////////////

        Clientdb db(connectDb);
		boost::asio::io_context io_context;
		Server s(io_context, serverPort);
		std::wcout << L"В адресной строке браузера, введите http://localhost:" << serverPort << std::endl;
		io_context.run();
	}
    catch (const pqxx::broken_connection& err)
    {
        consoleCol(col::br_red);
        std::wcerr << L"\nОшибка типа: " << typeid(err).name() << "\n";
        std::wcerr << ansi2wideUtf(err.what()) << '\n';
        consoleCol(col::cancel);
        return EXIT_FAILURE;
    }
    catch (const std::exception& err)
    {
        consoleCol(col::br_red);
        std::wcerr << L"\nИсключение типа: " << typeid(err).name() << '\n';
        std::wcerr << utf82wideUtf(err.what()) << '\n';
        consoleCol(col::cancel);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
