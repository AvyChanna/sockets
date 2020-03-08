#include "main.h"

#include "helpers.h"
#include "list_ops.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void *connection_handler();

pthread_mutex_t thread_info_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t critical_section_mutex = PTHREAD_MUTEX_INITIALIZER;

int socket_desc, client_sock, c;

thread_info_t thread_info[MAX_CLIENTS];

list_t buy_queue[MAX_ITEMS];
list_t sell_queue[MAX_ITEMS];
list_t ledger;
const char delim[2] = {';', 0};
const char *login_info[MAX_TRADERS] = {
// Compile time constant initialization
#define STATIC_INIT_VALUE "a"
#define STATIC_INIT_COUNT MAX_TRADERS
#include "static_init.h"
#undef STATIC_INIT_VALUE
#undef STATIC_INIT_COUNT
};
const char *error_str = "ERROR";
const char *ok_str = "OK";

void init_data_structures(void);

void init_data_structures(void) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		thread_info[i].thread_busy = 0;
	}
	ledger.size = 0;
	for(int i = 0; i < MAX_ITEMS; i++) {
		buy_queue[i].size = 0;
		sell_queue[i].size = 0;
	}
}

// main driver program
int main(int argc, char *argv[]) {
	init_data_structures();
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
	signal(SIGINT, sigintHandler);
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
			continue;
		}
		printf("got free id = %d\n", t);
		thread_info[t].client_socket_des = client_sock;
		thread_info[t].client = client;
		if(pthread_create(&(thread_info[t].thread_id), NULL, connection_handler, NULL) < 0) {
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
void *connection_handler(void) {
	// Get the socket descriptor
	int t = get_int_from_id(pthread_self());
	int sock = thread_info[t].client_socket_des;
	// thread_info[t].user_id = -1;
	// ^NOT reqd because get_free_id does this for us
	int read_size;
	char client_message[BUFFER_SIZE];
	char *ptr = client_message;
	// char buffer[BUFFER_SIZE], buffer2[BUFFER_SIZE];

	// Receive a message from client
	while((read_size = recv(sock, client_message, BUFFER_SIZE, 0)) > 0) {
		client_message[read_size] = '\0';
		char *save_ptr, *dummy;
		ptr = strtok_r(client_message, delim, &save_ptr);
		int func = strtol(ptr, &dummy, 10);
		if(func != 0 && thread_info[t].user_id == -1) {
			write(sock, error_str, BUFFER_SIZE);
			// memset(client_message, 0, BUFFER_SIZE);
			close(sock);
			set_free_by_id(pthread_self());
			return NULL;
		}
		print("FUNC=%d", func);
		// WIP: Process message here
		////////////////////////////////////////////////////////////////////////////
		// client message is in client_message[_BUFFER_SIZE];
		// Main business logic goes here
		if(func == 0) {		   // 0:login_id:password:NULL
			// char buffer[BUFFER_SIZE], buffer2[BUFFER_SIZE];
			int login_id = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			char *password = strtok_r(NULL, delim, &save_ptr);
			print("login=%d, pw=%s", login_id, password);
			pthread_mutex_lock(&thread_info_mutex);
			int flag = check_login(login_id, password, t);
			pthread_mutex_unlock(&thread_info_mutex);
			if(!flag) {
				// (login_id > 10 || login_id < 1) || strcmp(password, login_info[login_id])
				write(sock, error_str, BUFFER_SIZE);
				// memset(client_message, 0, BUFFER_SIZE);
				close(sock);
				set_free_by_id(pthread_self());
				return NULL;
			}
			write(sock, ok_str, BUFFER_SIZE);
			thread_info[t].user_id = login_id;
			continue;
			// memset(client_message, 0, BUFFER_SIZE);
		} else if(func == 1) {
			int item_code = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			int quantity = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			int unit_price = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			pthread_mutex_lock(&critical_section_mutex);
			int flag = add_buy_order(thread_info[t].user_id, item_code, quantity, unit_price);
			pthread_mutex_unlock(&critical_section_mutex);
			if(!flag) {
				write(sock, error_str, BUFFER_SIZE);
				// memset(client_message, 0, BUFFER_SIZE);
				close(sock);
				set_free_by_id(pthread_self());
				return NULL;
			}
			write(sock, ok_str, BUFFER_SIZE);
			continue;
		} else if(func == 2) {
			int item_code = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			int quantity = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			int unit_price = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			pthread_mutex_lock(&critical_section_mutex);
			int flag = add_sell_order(thread_info[t].user_id, item_code, quantity, unit_price);
			pthread_mutex_unlock(&critical_section_mutex);
			if(!flag) {
				write(sock, error_str, BUFFER_SIZE);
				// memset(client_message, 0, BUFFER_SIZE);
				close(sock);
				set_free_by_id(pthread_self());
				return NULL;
			}
			write(sock, ok_str, BUFFER_SIZE);
			continue;
		} else if(func == 3) {
			entry_t temp[MAX_ITEMS][2];
			for(int i = 0; i < MAX_ITEMS; i++) {
				pthread_mutex_lock(&critical_section_mutex);
				list_get_min(&buy_queue[i], &(temp[i][0]));
				list_get_max(&buy_queue[i], &(temp[i][1]));
				pthread_mutex_unlock(&critical_section_mutex);
			}
			char b[10] = {0};
			sprintf("%d%c", sizeof(temp), 0);
			write(sock, b, 10);
			sleep(1);
			write(sock, temp, sizeof(temp));
		} else if(func == 4) {
			record_t temp[MAX_TRANS];
			int c = 0;
			pthread_mutex_lock(&critical_section_mutex);
			for(int i = ledger_size(&ledger); i >= 0; i--) {
				record_t r = ledger_get_record(&ledger, i);
				if(r.buyer == thread_info[i].user_id || r.seller == thread_info[i].user_id)
					temp[c++] = r;
			}
			pthread_mutex_unlock(&critical_section_mutex);
			char b[10] = {0};
			sprintf("%d%c", sizeof(temp), 0);
			write(sock, b, 10);
			sleep(1);
			write(sock, temp, sizeof(temp));
		} else {
			write(sock, error_str, BUFFER_SIZE);
			close(sock);
			set_free_by_id(pthread_self());
			return NULL;
		}
	}

	if(read_size == 0) {
		printf("Client disconnected\n");
		fflush(stdout);
	} else if(read_size == -1) {
		perror("recv failed\n");
	}
	close(sock);
	set_free_by_id(pthread_self());
	return NULL;
}
