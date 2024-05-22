CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = webserv

SRCS =	Server/WebServer.cpp \
		Manager/Manager.cpp \
		request/Client.cpp \
		request/RequestHandle.cpp \
		request/Request.cpp \
		request/ResponseHandle.cpp \
		request/Response.cpp \
		request/NResponseUtils.cpp \
		socket/socket.cpp \
		Parse/Config.cpp \
		Parse/ServerConfig.cpp \
		Parse/LocationConfig.cpp \
		main/main.cpp \
		utils/utils.cpp \
		utils/Error.cpp \

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

.cpp.o: 
	${CC} $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)

re:	
	@$(MAKE) fclean
	@$(MAKE) all

.PHONY: all bonus clean fclean re
