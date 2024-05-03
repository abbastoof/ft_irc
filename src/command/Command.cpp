#include "../server/Server.h"
#include "Command.h"
#include "../debug/debug.h"

Command::Command(Server *server_ptr) : server_(server_ptr)
{
}

Command::~Command()
{
}

/**
 * @brief sends back a PONG when client sends a PING
 * when client sends a PING it contains a token, which has to be sent back to the client
 * this is done in order to track the latency between client and server
 * TODO: keeping track of PING PONG status, time of receiving message to the client class?
 * @param msg
 */
void Command::handlePing(const Message &msg)
{
	std::shared_ptr<Client> client_ptr = msg.getClientPtr();
	int fd = client_ptr->getFd();
	std::vector<std::string> parameters = msg.getParameters();
	if (parameters.empty())
	{
		server_->send_response(fd, ERR_NEEDMOREPARAMS(client_ptr->getClientPrefix(), "PING"));
	}
	server_->send_response(fd, PONG(server_->getServerHostname(), parameters.front())); // latter parameter is the token received from client
}

bool Command::channelExists(std::string const &channel_name)
{
	return server_->findChannel(channel_name) != nullptr;
}

void Command::handleCap(const Message &msg)
{
	(void)msg;

	// Implementation for CAP command
	return;
}
