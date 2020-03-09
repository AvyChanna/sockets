#include "helpers.h"

#include "list_ops.h"
#include "main.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// runs at exit for thread_exits and cleanup
void shutting_down(void) {
	// Now join the thread, so that we dont terminate before the thread
	pthread_mutex_lock(&thread_info_mutex);
	printf("Closing threads(if any)\n");
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(thread_info[i].thread_busy) {
			pthread_cancel((thread_info[i].thread_id));
		}
	}
	pthread_mutex_unlock(&thread_info_mutex);
	int truee = 1;
	printf("Closing main server socket\n");
	setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &truee, sizeof(int));
	printf("Destroying Mutex\n");
	pthread_mutex_destroy(&critical_section_mutex);
	pthread_mutex_destroy(&thread_info_mutex);
}

// find a free pthread_id and return it
int get_free_id(void) {
	int ret = -1;
	pthread_mutex_lock(&thread_info_mutex);
	printf("thread_busy = ");
	for(int i = 0; i < MAX_CLIENTS; i++)
		printf("%d ", thread_info[i].thread_busy);
	printf("\n");
	for(int t = 0; t < MAX_CLIENTS; t++)
		if(thread_info[t].thread_busy == 0) {
			thread_info[t].user_id = -1;
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

// close everything
void sigintHandler(int sig_num) {
	int res = 0;
	printf("\nCaught signal-%d. Choose->\n", sig_num);
	printf("0. Exit\n");
	printf("1. Print thread_busy\n");
	printf("2. Print buy_queue\n");
	printf("3. Print sell_queue\n");
	printf("4. Print ledger\n");

	printf("=>");
	scanf(" %d", &res);
	if(res == 0) {
		printf("Shutting down...\n");
		exit(0);
	} else if(res == 1) {
		printf("Thread_busy = ");
		for(int i = 0; i < MAX_CLIENTS; i++)
			printf("%d, ", thread_info[i].thread_busy);
		printf("\n");
	} else if(res == 2) {
		printf("===Buy queue===\n");
		for(int g = 0; g < MAX_ITEMS; g++) {
			int size = list_size(&buy_queue[g]);
			if(size) {
				printf("-Item %d-\n", g);
				for(int i = 0; i < size; i++)
					printf("SNo=%d,\tQ=%d,\tUP=%d,\tUSER=%d.\n", i, buy_queue[g].entry[i].quantity,
						   buy_queue[g].entry[i].unit_price, buy_queue[g].entry[i].user);
			}
		}
	} else if(res == 3) {
		printf("===Sell queue===\n");
		for(int g = 0; g < MAX_ITEMS; g++) {
			int size = list_size(&sell_queue[g]);
			if(size) {
				printf("-Item %d-\n", g);
				for(int i = 0; i < size; i++)
					printf("SNo=%d,\tQ=%d,\tUP=%d,\tUSER=%d.\n", i, sell_queue[g].entry[i].quantity,
						   sell_queue[g].entry[i].unit_price, sell_queue[g].entry[i].user);
			}
		}
	} else if(res == 4) {
		printf("===Ledger===\n");
		int size = ledger_size(&ledger);
		if(size)
			for(int i = 0; i < size; i++) {
				printf("SNo=%d,\tQ=%d,\tUP=%d,\tB=%d,\tS=%d.\n", i, ledger.record[i].quantity,
					   ledger.record[i].unit_price, ledger.record[i].buyer, ledger.record[i].seller);
			}
		else
			printf("Ledger Empty\n");
	}
}

void debug_print(const char *file, int line, const char *function, const char *message, ...) {
	va_list args;
	file = strrchr(file, '/') ? strrchr(file, '/') + 1 : file;
	printf("DEBUG:%s:%d in %s(): ", file, line, function);

	va_start(args, message);
	vprintf(message, args);
	printf("\n");
	va_end(args);
}

int check_login(int login_id UNUSED, char *password UNUSED, int t UNUSED) {
	if(login_id < 1 || login_id > 10)
		return 0;
	if(strcmp(password, login_info[login_id - 1]))
		return 0;
	int flag = 1;
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(thread_info[i].thread_busy && thread_info[i].user_id == login_id)
			flag = 0;
	return flag;
};
int add_buy_order(int login_id UNUSED, int item_code UNUSED, int quantity UNUSED, int unit_price UNUSED) {
	// WIP
	if(list_is_full(buy_queue))
		return 0;
	entry_t buy_entry = {.user = login_id, .quantity = quantity, .unit_price = unit_price};
	// if()

	return 1;
}
int add_sell_order(int login_id UNUSED, int item_code UNUSED, int quantity UNUSED, int unit_price UNUSED) {
	// TODO Sell_orders code
	return 1;
}
int write_to_socket(int sock, char *text) {
	return send(sock, text, BUFFER_SIZE, 0);
}
int read_from_socket(int sock, char *buffer) {
	return recv(sock, buffer, BUFFER_SIZE, 0);
}