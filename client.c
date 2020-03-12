
// Client side C/C++ program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SEPARATOR ';'
#define BUFFER_SIZE (100)
#define MAX_ITEMS (10)
#define PASS_BUFF_SIZE (50)
static char send_buffer[BUFFER_SIZE];
static char recv_buffer[BUFFER_SIZE];
static int sock = 0;
static int trader = -1;
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

void login();
void handle_conn();
void print_recv_buffer();
void print_send_buffer();
void place_order(int choice);
void order_status();
void ledger_status();

void shutting_down() {
	int truee = 1;
	close(sock);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &truee, sizeof(int));
}

int main(int argc, char const *argv[]) {
	argc--;
	argv++;
	struct sockaddr_in serv_addr;
	int port_number = 8080;
	char ip[100] = {0};
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
		printf("Missing IP, defaulting to 127.0.0.1\n");
		strcpy(ip, "127.0.0.1");
	}

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket creation error\n");
		return 0;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_number);
	int is_valid_ip = 1;

	// Convert IPv4 and IPv6 addresses from text to binary form
	if((is_valid_ip = inet_pton(AF_INET, ip, &serv_addr.sin_addr)) <= 0) {
		printf("Error connecting to IP\n");
	}
	if(!is_valid_ip && strcmp(ip, "127.0.0.1") == 0){
		printf("Exiting...\n");
		return 0;
	}
	if(!is_valid_ip) {
		printf("Falling back to 127.0.0.1\n");
		strcpy(ip, "127.0.0.1");
		is_valid_ip = inet_pton(AF_INET, ip, &serv_addr.sin_addr);
	}
	if(!is_valid_ip) {
		printf("Could not connect, exiting...");
		return 0;
	}
	atexit(shutting_down);
	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed for server at %s:%d\n", ip, port_number);
		return 0;
	}
	login();
	handle_conn();
	return 0;
}

void print_recv_buffer() {
	// printf("Response=%s\n", recv_buffer);
}

void print_send_buffer() {
	// printf("Sent=%s\n", send_buffer);
}

void login() {
	char password[PASS_BUFF_SIZE];
	printf("Enter Tader ID=");
	scanf(" %d", &trader);
	trader--;
	printf("Enter password=");
	scanf(" %s", password);
	password[PASS_BUFF_SIZE] = 0;
	if(password[strlen(password) - 1] == '\n')
		password[strlen(password) - 1] = 0;
	// 0:login_id:password:NULL
	snprintf(send_buffer, BUFFER_SIZE, "0%c%d%c%s%c", SEPARATOR, trader, SEPARATOR, password, 0);
	send_buffer[BUFFER_SIZE - 1] = 0;
	print_send_buffer();
	if(send(sock, send_buffer, BUFFER_SIZE, 0) == -1) {
		perror("Error Logging in");
		exit(0);
	}
	if(recv(sock, recv_buffer, BUFFER_SIZE, 0) == -1) {
		perror("Error Logging in\n");
		exit(0);
	}
	recv_buffer[BUFFER_SIZE - 1] = 0;
	if(strcmp(recv_buffer, "OK")) {
		printf("Error Logging in\n");
		print_recv_buffer();
		exit(0);
	}
	printf("LOGGED IN\n");
}

void handle_conn() {
	int choice = -1;
	while(1) {
		printf("1.Send buy request\n");
		printf("2.Send sell request\n");
		printf("3.View order status\n");
		printf("4.View trade status\n");
		printf("*.Exit\n");
		printf("Input=");
		scanf(" %d", &choice);
		if(choice == 1 || choice == 2)
			place_order(choice);
		else if(choice == 3)
			order_status();
		else if(choice == 4)
			ledger_status();
		else
			exit(0);
		printf("\n");
	}
}

void place_order(int choice) {
	int item, unit_price, quantity;
	printf("Enter Item Number=");
	scanf(" %d", &item);
	item--;
	printf("Enter Quantity=");
	scanf(" %d", &quantity);
	printf("Enter unit price=");
	scanf(" %d", &unit_price);
	snprintf(send_buffer, BUFFER_SIZE, "%d%c%d%c%d%c%d%c", choice, SEPARATOR, item, SEPARATOR, quantity, SEPARATOR,
			 unit_price, 0);
	print_send_buffer();
	if(send(sock, send_buffer, BUFFER_SIZE, 0) == -1) {
		perror("Error placing order");
		exit(0);
	}
	if(recv(sock, recv_buffer, BUFFER_SIZE, 0) == -1) {
		perror("Error placing order\n");
		exit(0);
	}
	recv_buffer[BUFFER_SIZE - 1] = 0;
	print_recv_buffer();
	if(strcmp(recv_buffer, "OK")) {
		printf("Error placing order\n");
		print_recv_buffer();
		exit(0);
	}
	if(choice == 1)
		printf("Buy order placed\n");
	else
		printf("Sell order placed\n");
}

void order_status() {
	snprintf(send_buffer, BUFFER_SIZE, "3%c", 0);
	send_buffer[BUFFER_SIZE - 1] = 0;
	print_send_buffer();
	if(send(sock, send_buffer, BUFFER_SIZE, 0) == -1) {
		perror("Error in order send");
		exit(0);
	}
	int s;
	if(recv(sock, &s, sizeof(int), 0) == -1) {
		perror("Recv error for b in order_status");
		exit(0);
	}
	if(s == 0) {
		printf("No orders\n");
		return;
	}
	entry_t *temp = (entry_t *)malloc(sizeof(entry_t) * s);
	if(temp == NULL) {
		perror("Temp memory alloc failed");
		exit(0);
	}
	if(recv(sock, temp, sizeof(entry_t) * s, 0) == -1) {
		perror("Recv error for temp in order_status");
		exit(0);
	}
	printf("MAX Buy Prices\n");
	entry_t data;
	for(int i = 0; i < MAX_ITEMS; i++) {
		data = *(temp + i);
		if(data.user >= 0)
			printf("I=%2d,\tU=%2d,\tQ=%d,\tUP=%d\n", i + 1, data.user + 1, data.quantity, data.unit_price);
		else
			printf("I=%2d,\t-----\n", i + 1);
	}
	printf("MIN Sell price\n");
	for(int i = 0; i < MAX_ITEMS; i++) {
		data = *(temp + i + 10);
		if(data.user >= 0)
			printf("I=%2d,\tU=%2d,\tQ=%d,\tUP=%d\n", i + 1, data.user + 1, data.quantity, data.unit_price);
		else
			printf("I=%2d,\t-----\n", i + 1);
	}
	free(temp);
}

void ledger_status() {
	snprintf(send_buffer, BUFFER_SIZE, "4%c", 0);
	send_buffer[BUFFER_SIZE - 1] = 0;
	print_send_buffer();
	if(send(sock, send_buffer, BUFFER_SIZE, 0) == -1) {
		perror("Error in order send");
		exit(0);
	}
	int s;
	if(recv(sock, &s, sizeof(int), 0) == -1) {
		perror("Recv error for b in ledger_status");
		exit(0);
	}
	if(s == 0) {
		printf("No transactions\n");
		return;
	}
	record_t *temp = (record_t *)malloc(sizeof(record_t) * s);
	if(temp == NULL) {
		perror("Temp memory alloc failed");
		exit(0);
	}
	if(recv(sock, temp, sizeof(record_t) * s, 0) == -1) {
		perror("Recv error for temp in ledger_status");
		exit(0);
	}
	record_t data;
	for(int i = 0; i < s; i++) {
		data = *(temp + i);
		printf("I=%d,\tB=%d,\tS=%d,\tQ=%d,\tUP=%d\n", i + 1, data.buyer + 1, data.seller + 1, data.quantity,
			   data.unit_price);
	}
	free(temp);
	temp = NULL;
}
