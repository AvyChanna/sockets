// gcc server.c -lpthread -o server

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define RECV_BUFFER_SIZE 100
void *connection_handler(void *);
void shutting_down();

pthread_t thread_id[5];
int thread_busy[5] = {0, 0, 0, 0, 0};
pthread_mutex_t thread_busy_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t critical_section_mutex = PTHREAD_MUTEX_INITIALIZER;

// find a free pthread_id and return it
int get_free_id() {
	int i;
	pthread_mutex_lock(&thread_busy_mutex);
	for(int t = 0; t < 5; t++)
		if(!thread_busy[t]) {
			thread_busy[t] = 1;
			i = t;
		}
	pthread_mutex_unlock(&thread_busy_mutex);
	return i;
}

// free a pthread_id
int set_free_id(int t) {
	pthread_cancel(thread_id[t]);
	pthread_mutex_lock(&thread_busy_mutex);
	thread_busy[t] = 0;
	pthread_mutex_unlock(&thread_busy_mutex);
}

// main driver program
int main(int argc, char *argv[]) {
	argc--;
	argv++;
	int port_number = 8080;
	if(argc) {
		port_number = atoi(argv[0]);
		printf("Port Number = %d\n", port_number);
	} else
		printf("No port given, assuming 8080");
	atexit(shutting_down);
	int socket_desc, client_sock, c;
	struct sockaddr_in server, client;
	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_desc == -1) {
		printf("Could not create socket");
		exit(1);
	}
	printf("Socket created at %d", port_number);

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port_number);

	// Bind
	if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("bind failed. Error");
		return 1;
	}
	printf("bind done");

	// Listen
	if(!listen(socket_desc, 5)) {
		perror("listen failed");
		return 1;
	}

	// Accept and incoming connection
	printf("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	// pthread_t thread_id; FIXED: Made global

	while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))) {
		printf("Connection accepted for type-%d client at %d:%d", client.sin_family, client.sin_addr, client.sin_port);

		if(pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sock) < 0) {
			perror("could not create thread");
			return 1;
		}
		printf("Handler assigned");
	}

	if(client_sock < 0) {
		perror("accept failed");
		return 1;
	}

	return 0;
}

// This will handle connection for each client
void *connection_handler(void *socket_desc) {
	// Get the socket descriptor
	int sock = *(int *)socket_desc;
	int read_size;
	char *message, client_message[RECV_BUFFER_SIZE];

	// Send some messages to the client
	message = "Greetings! I am your connection handler\n";
	write(sock, message, strlen(message));

	message = "Now type something and i shall repeat what you type \n";
	write(sock, message, strlen(message));

	// Receive a message from client
	while((read_size = recv(sock, client_message, 2000, 0)) > 0) {
		client_message[read_size] = '\0';
		pthread_mutex_lock(&critical_section_mutex);
		// WIP: Process message here
		////////////////////////////////////////////////////////////////////////////
		// client message is in client_message[2000];
		// Main business logic goes here
		if(client_message[0] == '0') {		  // 0:login_id:password:NULL
			char buffer[2000], buffer2[2000];
			int login_id = client_message[1] - '0';
			int i = 2;
			for(; client_message[i] != ':'; i++) {
				buffer[i - 2] = client_message[i];
			}
			buffer[i - 2] = 0;
			if(login_id >= 0 && login_id <= 9)
				sprintf(buffer2, "Login_id = %d, Password = %s", login_id, buffer);
			pthread_mutex_unlock(&critical_section_mutex);
			write(sock, buffer2, strlen(buffer2));
			memset(client_message, 0, 2000);
			return;
		}

		////////////////////////////////////////////////////////////////////////////
		// pthread_mutex_unlock(&critical_section_mutex);
		// write(sock, client_message, strlen(client_message));
		// memset(client_message, 0, 2000);
	}

	if(read_size == 0) {
		printf("Client disconnected");
		fflush(stdout);
	} else if(read_size == -1) {
		perror("recv failed");
	}

	return 0;
}

// runs at exit for thread_exits and cleanup
void shutting_down() {
	// Now join the thread, so that we dont terminate before the thread
	for(int i = 0; i < 5; i++)
		if(thread_busy[i])
			pthread_join(thread_id[i], NULL);
	pthread_mutex_destroy(&critical_section_mutex);
	pthread_mutex_destroy(&thread_busy_mutex);
}
