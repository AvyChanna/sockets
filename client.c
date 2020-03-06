
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
	char send_buffer[200] = "01:qwerty";
	char buffer[200] = {0};
	int port_number = 8080;
	char ip[100] = {0};
	if(argv >= 2) {
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
		printf("Missing IP, defaulting to 127.0.0.1\n");
		return 1;
	}
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket creation error\n");
		return 1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_number);
	int is_valid_ip = 1;

	// Convert IPv4 and IPv6 addresses from text to binary form
	if((is_valid_ip = inet_pton(AF_INET, ip, &serv_addr.sin_addr)) <= 0) {
		printf("Invalid address/ Address not supported, exiting...\n");
		return 1;
	}
	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed for server at %s:%d\n", ip, port_number);
		return 1;
	}
	int valsend;
	while(1) {
		if((valsend = send(sock, send_buffer, strlen(send_buffer), 0)) == -1) {
			perror("asdfghjkl\n");
			printf("Could not send data\n");
			return 1;
		}
		printf("Sent=%d:%s\n", valsend, send_buffer);
		valread = read(sock, buffer, strlen(buffer));
		perror("qwertyuiop\n");
		printf("Reply=%d: %s\n", valread, buffer);
		sleep(1);
	}
	return 0;
}
