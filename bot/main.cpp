#include "Bot.h"
#include "../src/headers.h"
#include "../src/server/Server.h"
void convertArgs(std::string &server_address, int &port, std::string &password, char **av)
{
	int index;
	server_address = av[1];
	for (index = 0; av[2][index];index++)
	{
		if (!std::isdigit(av[2][index]))
			throw std::runtime_error("Port must be a digit");
	}
	port = std::stoi(av[2]);
	if (port < 0 || (port >= 0 && port <= 1024) || port > 65536)
		throw std::runtime_error("Invalid port number, must be between 1025 and 65536");
	if (av[3])
		password = av[3];
}

int main(int ac, char **av)
{
	int 		port=-1;
	std::string server_address;
	std::string password;
	try
	{
		switch (ac)
		{
			case 1:
				port = DEFAULTPORT;
				server_address = "localhost";
				break;
			case 2:
			case 3:
			case 4:
				convertArgs(server_address, port, password, av);
				break;
			default:
				std::cout << "Usage:\n./ircserv <Port> <Password>";
				break ;
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
		return (1);
	}
	try
	{
		Bot bot(server_address, port, password);
		// std::signal(SIGINT, Bot::signalhandler);
		// std::signal(SIGQUIT, Bot::signalhandler);
		bot.init_bot();
	}
	catch(std::exception &exp)
	{
		std::cout << exp.what() << std::endl;
		return 0;
	}
	
}