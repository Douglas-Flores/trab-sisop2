# build the server executable
all: server_tcp.c 
	gcc -g -Wall -o server_app server_tcp.c com_manager.c -lpthread

clean: 
	$(RM) server_app
