#ifndef _MAIN_
#define _MAIN_

#include "main.h"

#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE (100)
#define MAX_CLIENTS (5)
#define MAX_RECORDS (50)
#define MAX_TRADERS (10)
#define MAX_ITEMS (10)
#define MAX_TRANS (10)
#define SEPARATOR (8)
#define DELIMITER ';'

extern const char delim[2];
typedef struct thread_info_t {		  // Thread and session info
	// int t;
	pthread_t thread_id;
	int thread_busy;
	int client_socket_des;
	struct sockaddr_in client;
	int user_id;
	// Add login info here
} thread_info_t;

typedef struct entry_t {
	int user;
	int quantity;
	int unit_price;
} entry_t;

typedef struct record_t {
	int buyer;
	int seller;
	int item;
	int quantity;
	int unit_price;
} record_t;

typedef struct list_t {
	entry_t entry[MAX_RECORDS];
	int size;
} list_t;

typedef struct ledger_t {
	record_t record[MAX_RECORDS];
	int size;
} ledger_t;

extern thread_info_t thread_info[MAX_CLIENTS];
extern pthread_mutex_t thread_info_mutex;
extern pthread_mutex_t critical_section_mutex;
extern char login_info[MAX_TRADERS][20];
extern int socket_desc, client_sock, c;
extern list_t buy_queue[MAX_ITEMS];
extern list_t sell_queue[MAX_ITEMS];
extern ledger_t ledger;

#endif
