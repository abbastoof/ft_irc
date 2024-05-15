#include "BotCommand.hpp"

BotCommand::BotCommand(Bot *bot_ptr)
{
	bot_ = bot_ptr;
}

BotCommand::~BotCommand()
{
}

void BotCommand::handleJoin(const BotMessage &msg)
{
	std::string reply_number = msg.getReplyNumber();
	int fd = bot_->getServerfd();
	if (reply_number == "451")
	{
		bot_->send_response(fd, RPL_NICK(bot_->getNickname()));
		bot_->send_response(fd, RPL_USER(bot_->getUsername()));
	}
}

void BotCommand::handleNick(const BotMessage &msg)
{
	std::string reply_number = msg.getReplyNumber();
	int fd = bot_->getServerfd();
	std::string nick = bot_->getNickname();
	if (reply_number == "433")
	{
		size_t pos = nick.find_last_of("0123456789");
		if(pos != std::string::npos)
		{
			std::string num_in_nick = nick.substr(pos);
			int number;
			std::stringstream(num_in_nick) >> number;
			++number;
			nick.replace(pos, num_in_nick.length(), std::to_string(number));
			bot_->setNickname(nick);
		}
		else
			nick += "_";
		bot_->send_response(fd, RPL_NICK(bot_->getNickname()));
	}
}

void BotCommand::handlePrivmsg(const BotMessage &msg)
{
	std::string prefix = msg.getPrefix();
	std::string channel_name = msg.getParameters()[0];
	if (prefix[0] == ':')
		prefix.erase(0, 1);
	size_t pos = prefix.find('!');
	std::string offender;
	if (pos != std::string::npos)
		offender = prefix.substr(0, pos);
	std::stringstream line(msg.getTrailer());
	int fd = bot_->getServerfd();
	std::string fbomb;
	while (line >> fbomb)
	{
		if (auto result = std::find(bot_->getFbombs().begin(), bot_->getFbombs().end(), fbomb) != bot_->getFbombs().end())
			bot_->send_response(fd, KICK_REQUEST(channel_name, offender + " :you are not allowed to use fbombs on this server"));
	}
}

void BotCommand::handleKick(const BotMessage &msg)
{
    (void)msg;
}

void BotCommand::handleInvite(const BotMessage &msg)
{
	std::string reply_number = msg.getReplyNumber();
	int fd = bot_->getServerfd();
	std::cout << "reply_number = " << reply_number << std::endl;
	if (reply_number == "INVITE" && msg.getParameters()[0] == bot_->getNickname())
	{
		bot_->send_response(fd, "JOIN " + msg.getTrailer() + CRLF);
	}
}