
// Client side C/C++ program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	argc--;
	argv++;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *hello = "01:qwerty";
	char buffer[1024] = {0};
	int port_number = 8080;
	char *ip = "127.0.0.1";
	if(argv >= 2) {
		if(atoi(argv[1]))
			port_number = atoi(argv[1]);
		else
			printf("Wrong port number, defaulting to 8080\n");
	} else
		printf("Missing port number, defaulting to 8080\n");
	if(argv)
		ip = argv[0];
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket creation error\n");
		return 1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_number);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		printf("Invalid address/ Address not supported");
		if((!strcmp(ip, "127.0.0.1")) && inet_hton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
			printf("Could not process 127.0.0.1, exiting\n");
			return 1;
		} else
			printf(", defaulting to 127.0.0.1\n");
	}

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed for server at %s:%d\n", ip, port_number);
		return 1;
	}
	send(sock, hello, strlen(hello), 0);
	printf("Hello message sent\n");
	valread = read(sock, buffer, 1024);
	printf("Reply: %s\n", buffer);
	return 0;
}
