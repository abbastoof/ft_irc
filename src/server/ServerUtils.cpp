#include "Server.h"

void Server::shutdownServer(const std::string &reason)
{
	std::cout << RED << reason << ", shutting down server..." RESET << std::endl;
}

void Server::signalHandler(int signum)
{
	switch (signum)
	{
	case SIGINT:
		shutdownServer("SIGINT (Interrupt signal)");
		break;
	case SIGQUIT:
		shutdownServer("SIGQUIT (Quit)");
		break;
	}
	signal_ = true;
}

void Server::deleteClient(int fd)
{
	auto iter = clients_.find(fd);
	if (iter != clients_.end())
		clients_.erase(iter);
}

void Server::closeDeletePollFd(int fd)
{
	for (auto index = fds_.begin(); index != fds_.end(); index++)
	{
		if (index->fd == fd)
		{
			close(index->fd);
			fds_.erase(index);
			break;
		}
	}
}

void Server::closeFds()
{
	std::cout << RED "Closing all connections" RESET << std::endl;
	if (fds_.size() > 1)
	{
		for (auto index = fds_.begin(); index != fds_.end(); index++)
		{
			if (index->fd)
			{
				close(index->fd);
				fds_.erase(index);
			}
		}
	}
}

std::shared_ptr<Client> Server::findClientUsingFd(int fd) const
{
	if (clients_.empty())
		return nullptr;
	auto iter = clients_.find(fd);
	if (iter != clients_.end())
		return iter->second;
	return nullptr;
}

/**
 * @brief for finding client using nickname
 * @param nickname
 * @return std::shared_ptr<Client>
 */
std::shared_ptr<Client> Server::findClientUsingNickname(std::string const &nickname) const
{
	if (clients_.empty())
		return nullptr;
	std::string lower_case_nickname = nickname;
	std::transform(lower_case_nickname.begin(), lower_case_nickname.end(), lower_case_nickname.begin(), ::tolower); // Convert the nickname to lowercase
	for (auto it = clients_.begin(); it != clients_.end(); it++)
	{
		std::string user_nick = it->second->getNickname();
		std::transform(user_nick.begin(), user_nick.end(), user_nick.begin(), ::tolower); // Convert the user's nickname to lowercase
		if (user_nick == lower_case_nickname)
			return it->second;
	}
	return nullptr;
}

/**
 * @brief for finding client using old nickname
 * @param nickname
 * @return std::shared_ptr<Client>
 */
std::shared_ptr<Client> Server::findClientUsingOldNickname(std::string const &nickname) const
{
	if (clients_.empty())
		return nullptr;
	std::string lower_case_nickname = nickname;
	std::transform(lower_case_nickname.begin(), lower_case_nickname.end(), lower_case_nickname.begin(), ::tolower); // Convert the nickname to lowercase
	for (auto it = clients_.begin(); it != clients_.end(); it++)
	{
		std::string user_nick = it->second->getOldNickname();
		std::transform(user_nick.begin(), user_nick.end(), user_nick.begin(), ::tolower); // Convert the user's nickname to lowercase
		if (user_nick == lower_case_nickname)
			return it->second;
	}
	return nullptr;
}

/**
 * @brief function for finding channel by name.
 * this search is currently case sensitive, which it probably should not be
 * TODO: convert names to lowercase for comparison
 * @param channel_name
 * @return std::shared_ptr<Channel>
 */
std::shared_ptr<Channel> Server::findChannel(std::string const &channel_name)
{
	if (channels_.empty())
		return nullptr;
	auto iter = channels_.find(channel_name);
	if (iter != channels_.end())
		return iter->second;
	return nullptr;
}

void Server::whoGotDisconnected(int fd)
{
	std::shared_ptr<Client> client = findClientUsingFd(fd);
	std::string clientidentity;
	if (client)
	{
		if ((clientidentity = client->getNickname()).empty())
			clientidentity = std::to_string(fd);
	}
}

char *Server::extractUserIpAddress(struct sockaddr_in6 usersocketaddress)
{
	char *ipstr = nullptr;

	if (usersocketaddress.sin6_family == AF_INET6)
	{
		if (IN6_IS_ADDR_V4MAPPED(&(usersocketaddress.sin6_addr)))
		{
			// It's an IPv4-mapped IPv6 address, extract the IPv4 address
			struct in_addr ipv4addr;
			memcpy(&ipv4addr, &(usersocketaddress.sin6_addr.s6_addr[12]), sizeof(struct in_addr));
			ipstr = new char[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &ipv4addr, ipstr, INET_ADDRSTRLEN);
		}
		else
		{
			// It's a regular IPv6 address

			ipstr = new char[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &(usersocketaddress.sin6_addr), ipstr, INET6_ADDRSTRLEN);
		}
	}
	else if (usersocketaddress.sin6_family == AF_INET)
	{
		// It's an IPv4 address
		ipstr = new char[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(usersocketaddress.sin6_addr), ipstr, INET_ADDRSTRLEN);
	}
	return ipstr;
}

void Server::send_response(int fd, const std::string &response)
{
	std::cout << "Response:\n"
			  << response;
	if (send(fd, response.c_str(), response.length(), 0) < 0)
		debug("Response send() faild", FAILED);
}

// getter for map of supported commands
std::map<std::string, void (Command::*)(const Message &msg)> const &Server::getSupportedCommands() const
{
	return supported_commands_;
}

void Server::setServerHostname()
{
	char hostname[256] = {};
	if (gethostname(hostname, 256) == 0)
		host_ = hostname;
	else
		debug("gethostname", FAILED);
	return;
}

const std::string &Server::getServerHostname() const
{
	return host_;
}

/// @brief because user needs to feel welcome, they need to receive a welcome message
/// @param fd
/// @param servername
/// @param nickname
/// @param client_prefix
void Server::welcomeAndMOTD(int fd, std::string const &servername, std::string const &nickname, std::string const &client_prefix)
{
	send_response(fd, RPL_CONNECTED(servername, nickname, client_prefix));
	send_response(fd, RPL_ISUPPORT(servername, nickname));
	send_response(fd, RPL_MOTDSTART(servername, nickname));
	// send_response(fd, RPL_MOTD(servername, nickname, "███████╗████████╗░░░░░░██╗██████╗░░█████╗░"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██╔════╝╚══██╔══╝░░░░░░██║██╔══██╗██╔══██╗"));
	// send_response(fd, RPL_MOTD(servername, nickname, "█████╗░░░░░██║░░░█████╗██║██████╔╝██║░░╚═╝"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██╔══╝░░░░░██║░░░╚════╝██║██╔══██╗██║░░██╗"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██║░░░░░░░░██║░░░░░░░░░██║██║░░██║╚█████╔╝"));
	// send_response(fd, RPL_MOTD(servername, nickname, "╚═╝░░░░░░░░╚═╝░░░░░░░░░╚═╝╚═╝░░╚═╝░╚════╝░"));
	// send_response(fd, RPL_MOTD(servername, nickname, " "));
	// send_response(fd, RPL_MOTD(servername, nickname, "░█████╗░░█████╗░███╗░░██╗██████╗░███████╗██╗░░░░░██╗███╗░░██╗"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██╔══██╗██╔══██╗████╗░██║██╔══██╗██╔════╝██║░░░░░██║████╗░██║"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██║░░██║███████║██╔██╗██║██║░░██║█████╗░░██║░░░░░██║██╔██╗██║"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██║░░██║██╔══██║██║╚████║██║░░██║██╔══╝░░██║░░░░░██║██║╚████║"));
	// send_response(fd, RPL_MOTD(servername, nickname, "╚█████╔╝██║░░██║██║░╚███║██████╔╝███████╗███████╗██║██║░╚███║"));
	// send_response(fd, RPL_MOTD(servername, nickname, "░╚════╝░╚═╝░░╚═╝╚═╝░░╚══╝╚═════╝░╚══════╝╚══════╝╚═╝╚═╝░░╚══╝"));
	// send_response(fd, RPL_MOTD(servername, nickname, " "));
	// send_response(fd, RPL_MOTD(servername, nickname, "███╗░░░███╗████████╗░█████╗░░█████╗░███████╗"));
	// send_response(fd, RPL_MOTD(servername, nickname, "████╗░████║╚══██╔══╝██╔══██╗██╔══██╗██╔════╝"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██╔████╔██║░░░██║░░░██║░░██║██║░░██║█████╗░░"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██║╚██╔╝██║░░░██║░░░██║░░██║██║░░██║██╔══╝░░"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██║░╚═╝░██║░░░██║░░░╚█████╔╝╚█████╔╝██║░░░░░"));
	// send_response(fd, RPL_MOTD(servername, nickname, "╚═╝░░░░░╚═╝░░░╚═╝░░░░╚════╝░░╚════╝░╚═╝░░░░░"));
	// send_response(fd, RPL_MOTD(servername, nickname, " "));
	// send_response(fd, RPL_MOTD(servername, nickname, "░█████╗░████████╗░█████╗░░█████╗░███████╗"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██╔══██╗╚══██╔══╝██╔══██╗██╔══██╗██╔════╝"));
	// send_response(fd, RPL_MOTD(servername, nickname, "███████║░░░██║░░░██║░░██║██║░░██║█████╗░░"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██╔══██║░░░██║░░░██║░░██║██║░░██║██╔══╝░░"));
	// send_response(fd, RPL_MOTD(servername, nickname, "██║░░██║░░░██║░░░╚█████╔╝╚█████╔╝██║░░░░░"));
	// send_response(fd, RPL_MOTD(servername, nickname, "╚═╝░░╚═╝░░░╚═╝░░░░╚════╝░░╚════╝░╚═╝░░░░░"));
	// send_response(fd, RPL_MOTD(servername, nickname, " "));
	send_response(fd, RPL_MOTD(servername, nickname, "\"Alright, let's see what we can see.. Everybody online, looking good.\""));
	send_response(fd, RPL_MOTD(servername, nickname, "Lieutenant Gorman to the marines before landing to terraforming colony on exomoon LV-426"));
	send_response(fd, RPL_MOTD(servername, nickname, "Aliens, 1986"));
	send_response(fd, RPL_MOTDEND(servername, nickname));
}

std::vector<std::shared_ptr<Client>> Server::findClientsByMask(const std::string &mask) const
{
	std::vector<std::shared_ptr<Client>> matchedClients;
	std::regex pattern(createRegexFromMask(mask)); // Convert mask to regex pattern, assuming you have a function to handle this

	for (const auto &clientPair : clients_)
	{
		std::string user_nick = clientPair.second->getNickname();
		if (std::regex_match(user_nick, pattern))
			matchedClients.push_back(clientPair.second);
	}
	return matchedClients;
}

std::string Server::createRegexFromMask(const std::string &mask) const
{
	std::string regex;
	regex.reserve(mask.size() * 2); // Reserve enough space to avoid frequent reallocations

	for (char ch : mask)
	{
		switch (ch)
		{
		case '*':
			regex.append(".*"); // Replace * with .* in the regex pattern (matches any sequence of characters)
			break;
		case '?':
			regex.append("."); // Replace ? with . in the regex pattern (matches any single character)
			break;
		case '\\':
		case '^':
		case '$':
		case '.':
		case '|':
		case '(':
		case ')':
		case '[':
		case ']':
		case '{':
		case '}':
		case '+':
			regex.append("\\"); // Escape special characters
		default:
			regex.push_back(ch); // Append the character as is to the regex pattern
			break;
		}
	}

	return regex;
}

std::string Server::toLower(const std::string& str) const
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lowerStr;
}
