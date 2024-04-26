#include "../server/Server.h"
#include "Command.h"

Command::Command(Server *server_ptr) : server_(server_ptr)
{
}

Command::~Command()
{
}

void Command::handleJoin(const Message &msg)
{
	std::cout << "handleJoin called" << std::endl;
	std::shared_ptr<Client> client_ptr = msg.getClientPtr();
	int fd = client_ptr->getFd();
	if (client_ptr->getRegisterStatus() == false)
	{
		server_->send_response(fd, ERR_NOTREGISTERED(server_->getServerHostname()));
		return;
	}
	std::vector<std::string> parameters = msg.getParameters();
	if (parameters.empty()) {
		server_->send_response(fd, ERR_NEEDMOREPARAMS(client_ptr->getClientPrefix(), "JOIN"));
		return;
	}
	std::string channel_name = parameters.front();
	if (server_->channelExists(channel_name) == true)
	{
		std::shared_ptr<Channel> channel_ptr = server_->findChannel(channel_name);
		//create a channel pointer for details from channel
		if (channel_ptr->isFull() == true)
		{
			server_->send_response(fd, ERR_CHANNELISFULL(channel_name));
			return;
		}
		if (channel_ptr->isInviteOnly() == true)
		{
			if (channel_ptr->isClientInvited(client_ptr->getNickname()) == false)
			{
				server_->send_response(fd, ERR_INVITEONLYCHAN(server_->getServerHostname(), client_ptr->getNickname(), channel_name));
				return;
			}
		}
		if (channel_ptr->isPasswordProtected() == true)
		{
			if (parameters.size() == 1) {
				server_->send_response(fd, ERR_NEEDMOREPARAMS(client_ptr->getClientPrefix(), "JOIN"));
				return;				
			}
			if (parameters.at(2) != channel_ptr->getPassword())
			{
				server_->send_response(fd, ERR_BADCHANNELKEY(channel_name));
				return;
			}
		}
		// in this section we actually join the channel
		// Implementation for JOIN command
	}
	else
	(
		// create a new channel, add client to the channel and set client as operator
	)
}

void Command::handleNick(const Message &msg)
{
	
	(void)msg;
	
	// Implementation for NICK command
	return;
}

void Command::handlePrivmsg(const Message &msg)
{
	(void)msg;
	
	// Implementation for PRIVMSG command
	return;
}

void Command::handleQuit(const Message &msg)
{
	(void)msg;
	
	// Implementation for QUIT command
	return;
}

void Command::handlePass(const Message &msg)
{
	std::shared_ptr<Client> client_ptr = msg.getClientPtr();
	if (server_->getPassword().empty())
		return;
	if (client_ptr->getRegisterStatus() == true)
	{
		server_->send_response(client_ptr->getFd(), ERR_ALREADYREGISTERED(client_ptr->getNickname()));
		return;
	}
	std::vector<std::string> parameters = msg.getParameters();
	size_t pos = parameters.front().find_first_not_of(" \t\v");
	
	if(pos == std::string::npos || parameters.empty())
		server_->send_response(msg.getClientfd(), ERR_NEEDMOREPARAMS(std::string("*"), "PASS"));
	else if (!client_ptr->getRegisterStatus())
	{
		if(parameters.front() == server_->getPassword()) 
		{
			// client->registerClient(); //This happens when the user has pass username and nick
			client_ptr->setPassword();
		}
		else if (!server_->getPassword().empty() && parameters.front() != server_->getPassword())
			server_->send_response(client_ptr->getFd(), ERR_INCORPASS(client_ptr->getNickname()));
	}
}

void Command::handleCap(const Message &msg)
{
	(void)msg;
	
	// Implementation for CAP command
	return;
}