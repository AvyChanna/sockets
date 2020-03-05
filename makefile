all: re

re: client.c server.c
	@gcc client.c -g -o client
	@gcc server.c -g -o server -lpthread
client: client.c
	@gcc client.c -g -o client

server:
	@gcc server.c -g -o server -lpthread
	
clean:
	@rm -f client server
