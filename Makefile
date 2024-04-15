# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: atoof <atoof@student.hive.fi>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/03 01:26:14 by atoof             #+#    #+#              #
#    Updated: 2024/04/03 01:26:14 by atoof            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ft_irc
SRCS = main.cpp Server.cpp Client.cpp
OBJS = $(SRCS:.cpp=.o)
SRC_DIR = src
OBJ_DIR = obj/
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++17
RM = rm -rf
HEADERS := $(wildcard $(shell find $(SRC_DIR) -type f -name "*.h"))

vpath %.cpp $(SRC_DIR)

all: $(OBJ_DIR) $(NAME)

$(NAME): $(addprefix $(OBJ_DIR), $(OBJS))
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
