# build the server executable
all: server_tcp.c 
	gcc -g -Wall -o app_cliente client_tcp.c com_manager.c -lpthread

clean: 
	$(RM) app_cliente