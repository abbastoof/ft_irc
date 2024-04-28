#include "Client.h"

Client::Client(const int &fd, const std::string &nickname, const std::string &username, const std::string &ipaddress) : fd_(fd), registered_(false), nickname_(nickname), username_(username), ip_address_(ipaddress)
{
}

Client::~Client()
{
}

// getters

int Client::getFd()
{
	return this->fd_;
}

std::string Client::getNickname()
{
	return this->nickname_;
}

std::string Client::getUsername()
{
	return this->username_;
}

std::string Client::getHostname()
{
	return this->hostname_;
}

std::string Client::getRealname()
{
	return this->realname_;
}

std::string Client::getIpAddress()
{
	return this->ip_address_;
}

bool Client::getRegisterStatus()
{
	return this->registered_;
}

char Client::getUserMode()
{
	return this->usermode_;
}

// setters

void Client::setFd(int const &fd)
{
	this->fd_ = fd;
}

/// 		nickname cannot be longer than 9 characters!
void Client::setNickname(std::string const &nickname)
{
	this->nickname_ = nickname;
}

void Client::setUsername(std::string const &username)
{
	this->username_ = username;
}

void Client::setHostname(std::string const &hostname)
{
	this->hostname_ = hostname;
}

void Client::setRealname(std::string const &realname)
{
	this->realname_ = realname;
}

void Client::setUserMode(char const &usermode)
{
	this->usermode_ = usermode;
}

void Client::setIpAddress(std::string const &ip_address)
{
	this->ip_address_ = ip_address;
}

void Client::registerClient()
{
	if (this->registered_ == false)
		this->registered_ = true;
}

void Client::unregisterClient()
{
	if (this->registered_ == true)
		this->registered_ = false;
}

void Client::processBuffer(Server *server_ptr)
{
	size_t pos;
	while ((pos = this->buffer.find("\r\n")) != std::string::npos)
	{
		std::string line = this->buffer.substr(0, pos);
		this->buffer.erase(0, pos + 2);
		if (!line.empty() && line.back() == '\r')
			line.pop_back(); // Remove the trailing \r
		//processCommand(line, this->fd_);
		
		Message message(line, server_ptr, this->fd_); // Parse the message
		if(message.isValidMessage() == true)
		{
			//TODO: send the message to command
			processCommand(message, server_ptr);
		}
	}
}

void Client::processCommand(Message &message, Server *server_ptr)
{
    const std::string &command = message.getCommand();
    //const std::vector<std::string> &params = message.getParameters();
    // const std::string &trailer = message.getTrailer();

    auto it = server_ptr->getSupportedCommands().find(command);
    if (it != server_ptr->getSupportedCommands().end())
    {
       auto handler = it->second; // Get the function pointer from the map
       Command commandObject(server_ptr);
       (commandObject.*handler)(message);
    }
}

void	Client::setPassword()
{
	password_ = true;
}

void Client::appendToBuffer(const std::string &data)
{
	this->buffer += data;
}

void Client::setClientPrefix()
{
	client_prefix_ = ":" + nickname_ + "!~" + username_ + "@" + ip_address_;
}

const std::string &Client::getClientPrefix()
{
	return client_prefix_;
}

const bool Client::hasSentPassword()
{
	return password_;
}
// this function is supposed to send a message to client
// void		Client::sendMessage(std::string const &message)
// {
// 	(void) message;
// }

// and this one would receive messages
// std::string	Client::receiveMessage()
// {

// }

// this would make the client join channel,
// maybe this should take a pointer to the channel
// instead of a string
// void		Client::joinChannel(std::string const &channel)
// {

// }