#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <string>
#include <iostream>

#include "http_connection.h"
#include <Windows.h>


void httpServer(tcp::acceptor& acceptor, tcp::socket& socket, EnterInfo &serverDB)
{
	acceptor.async_accept(socket,
		[&](beast::error_code ec)
		{
			if (!ec)
				std::make_shared<HttpConnection>(std::move(socket))->start(serverDB);
			httpServer(acceptor, socket, serverDB);
		});
}


int main(int argc, char* argv[])
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	try
	{
		EnterInfo serverDB("../../../../info_ini.txt");
		auto const address = net::ip::make_address("0.0.0.0");
		
		unsigned short port = serverDB.getPortNumber();
		net::io_context ioc{1};

		tcp::acceptor acceptor{ioc, { address, port }};
		tcp::socket socket{ioc};
		httpServer(acceptor, socket, serverDB);

		std::cout << "Open browser and connect to http://localhost:8080 to see the web server operating" << std::endl;

		ioc.run();
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}