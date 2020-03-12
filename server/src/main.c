#include "main.h"

#include "helpers.h"
#include "list_ops.h"

#include <arpa/inet.h>
#include <errno.h>
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
ledger_t ledger;
const char delim[2] = {DELIMITER, 0};
char login_info[MAX_TRADERS][20] = {0};
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

	FILE *pw = fopen("passwords.txt", "r");
	if(!pw) {
		debug("passwords.txt not opened: %s", DEBUG_ERR_STR);
		exit(0);
	}
	for(int i = 0; i < MAX_TRADERS; i++) {
		fgets(login_info[i], 20 * sizeof(char), pw);
		if(login_info[i][strlen(login_info[i]) - 1] == '\n')
			login_info[i][strlen(login_info[i]) - 1] = 0;
	}
	fclose(pw);
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
	signal(SIGINT, sigint_handler);
	struct sockaddr_in server, client;
	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_desc == -1) {
		debug("Could not create socket: %s", DEBUG_ERR_STR);
		return 0;
	}
	debug("Socket created at %d", port_number);

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port_number);

	// Bind
	if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		debug("bind failed: %s", DEBUG_ERR_STR);
		int truee = 1;
		close(socket_desc);
		setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &truee, sizeof(int));
		return 0;
	}
	debug("bind done\n");

	// Listen
	if(listen(socket_desc, MAX_CLIENTS)) {
		debug("listen failed");
		int truee = 1;
		close(socket_desc);
		setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &truee, sizeof(int));
		return 0;
	}

	// Accept and incoming connection
	debug("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	// pthread_t thread_id; FIXED: Made global
	int t;
	while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))) {
		debug("Connection accepted for type-%d client at %s:%d\n", client.sin_family, inet_ntoa(client.sin_addr),
			  client.sin_port);
		if((t = get_free_id()) == -1) {
			debug("%d threads already queued. Refusing...", MAX_CLIENTS);
			close(client_sock);
			continue;
		}
		debug("got free id = %d\n", t);
		thread_info[t].client_socket_des = client_sock;
		thread_info[t].client = client;
		if(pthread_create(&(thread_info[t].thread_id), NULL, connection_handler, NULL) < 0) {
			debug("could not create thread: %s", DEBUG_ERR_STR);
			return 0;
		}
		debug("Handler assigned");

		if(client_sock < 0) {
			debug("accept failed: %s", DEBUG_ERR_STR);
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
	// ^^^ NOT reqd because get_free_id does this for us
	int read_size;
	char client_message[BUFFER_SIZE];
	char *ptr = client_message;

	// Receive a message from client
	while((read_size = recv(sock, client_message, BUFFER_SIZE, 0)) > 0) {
		client_message[read_size] = '\0';
		char *save_ptr, *dummy;
		ptr = strtok_r(client_message, delim, &save_ptr);
		int func = strtol(ptr, &dummy, 10);
		if(func != 0 && thread_info[t].user_id == -1) {
			send(sock, error_str, BUFFER_SIZE, 0);
			// memset(client_message, 0, BUFFER_SIZE);
			close(sock);
			set_free_by_id(pthread_self());
			return NULL;
		}
		debug("FUNC=%d", func);
		if(func == 0) {		   // 0:login_id:password
			// char buffer[BUFFER_SIZE], buffer2[BUFFER_SIZE];
			int login_id = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			char *password = strtok_r(NULL, delim, &save_ptr);
			debug("login=%d, pw=%s", login_id, password);
			debug("LOCK thread");
			pthread_mutex_lock(&thread_info_mutex);
			int flag = check_login(login_id, password, t);
			if(flag)
				thread_info[t].user_id = login_id;
			debug("UNLOCK thread");
			pthread_mutex_unlock(&thread_info_mutex);
			debug("Login Success = %d", flag);
			if(!flag) {
				send(sock, error_str, BUFFER_SIZE, 0);
				close(sock);
				set_free_by_id(pthread_self());
				return NULL;
			}
			send(sock, ok_str, BUFFER_SIZE, 0);
			continue;
		} else if(func == 1) {		  // 1:item_code:quantity:unit_price
			int item_code = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			int quantity = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			int unit_price = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			debug("Buy I=%d,\tQ=%d,\tUP=%d.", item_code, quantity, unit_price);
			debug("LOCK data");
			pthread_mutex_lock(&critical_section_mutex);
			int flag = add_buy_order(thread_info[t].user_id, item_code, quantity, unit_price);
			debug("UNLOCK data");
			pthread_mutex_unlock(&critical_section_mutex);
			debug("Buy Order Success = %d", flag);
			if(!flag) {
				send(sock, error_str, BUFFER_SIZE, 0);
				close(sock);
				set_free_by_id(pthread_self());
				return NULL;
			}
			send(sock, ok_str, BUFFER_SIZE, 0);
			continue;
		} else if(func == 2) {		  // 2:item_code:quantity:unit_price
			int item_code = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			int quantity = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			int unit_price = strtol(strtok_r(NULL, delim, &save_ptr), &dummy, 10);
			debug("Sell I=%d,\tQ=%d,\tUP=%d.", item_code, quantity, unit_price);
			debug("LOCK data");
			pthread_mutex_lock(&critical_section_mutex);
			int flag = add_sell_order(thread_info[t].user_id, item_code, quantity, unit_price);
			debug("UNLOCK data");
			pthread_mutex_unlock(&critical_section_mutex);
			debug("Sell Order Success = %d", flag);
			if(!flag) {
				send(sock, error_str, BUFFER_SIZE, 0);
				close(sock);
				set_free_by_id(pthread_self());
				return NULL;
			}
			send(sock, ok_str, BUFFER_SIZE, 0);
			continue;
		} else if(func == 3) {		  // 3
			entry_t temp[MAX_ITEMS * 2];
			int dummy;
			debug("LOCK data");
			pthread_mutex_lock(&critical_section_mutex);
			for(int i = 0; i < MAX_ITEMS; i++) {
				list_get_max_price(&buy_queue[i], &(temp[i]), &dummy);
				list_get_min_price(&sell_queue[i], &(temp[i + MAX_ITEMS]), &dummy);
			}
			debug("UNLOCK data");
			pthread_mutex_unlock(&critical_section_mutex);
			int s = MAX_ITEMS * 2;
			send(sock, &s, sizeof(int), 0);
			// sleep(1);
			send(sock, temp, sizeof(entry_t) * s, 0);
		} else if(func == 4) {		  // 4
			record_t temp[MAX_TRANS];
			int c = 0;
			debug("LOCK data");
			pthread_mutex_lock(&critical_section_mutex);
			for(int i = ledger_size(&ledger); (i >= 0) && (c < MAX_TRANS); i--) {
				record_t r = ledger_get_record(&ledger, i);
				if(r.buyer == thread_info[i].user_id || r.seller == thread_info[i].user_id)
					temp[c++] = r;
			}
			debug("UNLOCK data");
			pthread_mutex_unlock(&critical_section_mutex);
			debug("Records returned = %d", c);
			int s = c;
			send(sock, &s, sizeof(int), 0);
			// sleep(1);		 // OPTIONAL !!!!!!!
			send(sock, (void *)temp, sizeof(record_t) * c, 0);
		} else {		// None of the above
			send(sock, error_str, BUFFER_SIZE, 0);
			close(sock);
			set_free_by_id(pthread_self());
			return NULL;
		}
	}

	if(read_size == 0) {
		debug("Client disconnected, read size = 0\n");
		fflush(stdout);
	} else if(read_size == -1) {
		debug("Recv failed, read size = -1: %s", DEBUG_ERR_STR);
	}
	close(sock);
	set_free_by_id(pthread_self());
	return NULL;
}
