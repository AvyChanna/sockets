#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define RECV_BUFFER_SIZE (100)
#define MAX_CLIENTS (5)
typedef struct thread_info_t {
	int t;
	pthread_t thread_id;
	int thread_busy;
	int client_socket_des;
	int user_id;
	// Add login info here
} thread_info_t;
thread_info_t thread_info[MAX_CLIENTS];
void *connection_handler(void *);
void shutting_down();
int get_free_id();
void set_free_by_int(int t);
void set_free_by_id(pthread_t t);
int get_int_from_id(pthread_t t);

pthread_mutex_t thread_info_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t critical_section_mutex = PTHREAD_MUTEX_INITIALIZER;

int socket_desc, client_sock, c;
// main driver program
int main(int argc, char *argv[]) {
	memset(thread_info, 0, sizeof(thread_info_t) * MAX_CLIENTS);
	for(int i = 0; i < MAX_CLIENTS; i++)
		thread_info[i].t = i;
	argc--;
	argv++;
	int port_number = 8080;
	if(argc) {
		if(atoi(argv[0])) {
			port_number = atoi(argv[0]);
			printf("Port Number = %d\n", port_number);
		} else
			printf("Wrong port number, defaulting to 8080\n");
	} else
		printf("No port given, assuming 8080\n");
	atexit(shutting_down);
	struct sockaddr_in server, client;
	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_desc == -1) {
		printf("Could not create socket\n");
		return 1;
	}
	printf("Socket created at %d\n", port_number);

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port_number);

	// Bind
	if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("bind failed. Error\n");
		return 1;
	}
	printf("bind done\n");

	// Listen
	if(listen(socket_desc, MAX_CLIENTS)) {
		printf("listen failed\n");
		return 1;
	}

	// Accept and incoming connection
	printf("Waiting for incoming connections...\n");
	c = sizeof(struct sockaddr_in);
	// pthread_t thread_id; FIXED: Made global
	int t;
	while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))) {
		printf("Connection accepted for type-%d client at %s:%d\n", client.sin_family, inet_ntoa(client.sin_addr),
			   client.sin_port);
		if((t = get_free_id()) == -1) {
			printf("%d threads already queued. Refusing...\n", MAX_CLIENTS);
			close(client_sock);
			break;
			continue;
		}
		printf("got free id = %d\n", t);
		if(pthread_create(&(thread_info[t].thread_id), NULL, connection_handler, (void *)&client_sock) < 0) {
			perror("could not create thread\n");
			return 1;
		}
		printf("Handler assigned\n");

		if(client_sock < 0) {
			perror("accept failed\n");
		}
	}
	return 0;
}

// This will handle connection for each client
void *connection_handler(void *socket_desc) {
	// Get the socket descriptor
	int sock = *(int *)socket_desc;
	int read_size;
	char *message, client_message[RECV_BUFFER_SIZE];

	// Receive a message from client
	while((read_size = recv(sock, client_message, RECV_BUFFER_SIZE, 0)) > 0) {
		client_message[read_size] = '\0';
		pthread_mutex_lock(&critical_section_mutex);
		// WIP: Process message here
		////////////////////////////////////////////////////////////////////////////
		// client message is in client_message[RECV_BUFFER_SIZE];
		// Main business logic goes here
		if(client_message[0] == '0') {		  // 0:login_id:password:NULL
			char buffer[RECV_BUFFER_SIZE], buffer2[RECV_BUFFER_SIZE];
			int login_id = client_message[1] - '0';
			int i = 3;
			for(; client_message[i] != 0; i++) {
				buffer[i - 3] = client_message[i];
			}
			buffer[i - 2] = 0;
			if(login_id >= 1 && login_id <= 10)
				sprintf(buffer2, "OK:Login_id = '%d', Password = '%s'%c", login_id, buffer, 0);
			pthread_mutex_unlock(&critical_section_mutex);
			write(sock, buffer2, strlen(buffer2));
			memset(client_message, 0, RECV_BUFFER_SIZE);
		}
		////////////////////////////////////////////////////////////////////////////
		// pthread_mutex_unlock(&critical_section_mutex);
		// write(sock, client_message, strlen(client_message));
		// memset(client_message, 0, RECV_BUFFER_SIZE);
	}

	if(read_size == 0) {
		printf("Client disconnected\n");
		fflush(stdout);
	} else if(read_size == -1) {
		perror("recv failed\n");
	}
	set_free_by_id(pthread_self());
	return NULL;
}

// runs at exit for thread_exits and cleanup
void shutting_down() {
	// Now join the thread, so that we dont terminate before the thread
	printf("Closing threads(if any)\n");
	pthread_mutex_lock(&thread_info_mutex);
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(thread_info[i].thread_busy) {
			thread_info[i].thread_busy = 0;
			pthread_cancel((thread_info[i].thread_id));
		}
	}
	pthread_mutex_unlock(&thread_info_mutex);
	int truee = 1;
	setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &truee, sizeof(int));
	pthread_mutex_destroy(&critical_section_mutex);
	pthread_mutex_destroy(&thread_info_mutex);
}

// find a free pthread_id and return it
int get_free_id() {
	int ret = -1;
	pthread_mutex_lock(&thread_info_mutex);
	printf("thread_busy = ");
	for(int i = 0; i < MAX_CLIENTS; i++)
		printf("%d ", thread_info[i].thread_busy);
	printf("\n");
	for(int t = 0; t < MAX_CLIENTS; t++)
		if(thread_info[t].thread_busy == 0) {
			thread_info[t].thread_busy = 1;
			ret = t;
			break;
		}
	pthread_mutex_unlock(&thread_info_mutex);
	return ret;
}

// free a pthread_id
void set_free_by_int(int t) {
	pthread_mutex_lock(&thread_info_mutex);
	thread_info[t].thread_busy = 0;
	pthread_mutex_unlock(&thread_info_mutex);
}

// free a pthread_id, NOT thread_safe
void set_free_by_id(pthread_t t) {
	set_free_by_int(get_int_from_id(t));
}

// get t from pthread_id, NOT thread_safe
int get_int_from_id(pthread_t t) {
	int mark;
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(thread_info[i].thread_id == t) {
			mark = i;
			break;
		}
	return mark;
}