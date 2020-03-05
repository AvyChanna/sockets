
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
	char ip[100];
	if(argc >= 2) {
		if(atoi(argv[1]))
			port_number = atoi(argv[1]);
		else
			printf("Wrong port number, defaulting to 8080\n");
	} else
		printf("Missing port number, defaulting to 8080\n");
	if(argc) {
		strncpy(ip, argv[0], 100);
		ip[99] = 0;
	} else {
		printf("No IP given\n");
		return 1;
	}
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket creation error\n");
		return 1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_number);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		printf("Invalid address/ Address not supported");
		return 1;
	}

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed for server at %s:%d\n", ip, port_number);
		return 1;
	}
	int valsend;
	while(1) {
		if((valsend = send(sock, hello, strlen(hello), 0)) == -1) {
			printf("Could not send data\n");
			return 1;
		}
		printf("Sent=%d:%s\n", valsend, hello);
		valread = read(sock, buffer, 1024);
		printf("Reply=%d: %s\n", valread, buffer);
		sleep(1);
	}
	return 0;
}
