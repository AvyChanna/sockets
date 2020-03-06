all: re

re:
	@gcc client.c -g -o client
	@gcc server.c -g -o server -lpthread
client:
	@gcc client.c -g -o client

server:
	@gcc server.c -g -o server -lpthread

clean:
	@rm -f client server
