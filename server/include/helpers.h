#ifndef _HELPERS_
#define _HELPERS_

#include "helpers.h"

#include <errno.h>
#include <pthread.h>


//****** Strict Type Checking *****//
#define UNUSED __attribute__((unused))
#define NO_RETURN __attribute__((noreturn))
#define NO_INLINE __attribute__((noinline))
#define PURE_FUNC __attribute__((const))

//****** DEBUG PRINT ******//
#define DEBUG_ERR_STR strerror(errno)
#define PRINTF_FORMAT(FMT, FIRST) __attribute__((format(printf, FMT, FIRST)))
#define SCANF_FORMAT(FMT, FIRST) __attribute__((format(scanf, FMT, FIRST)))
#define debug(...) debug_print(__FILE__, __LINE__, __func__, __VA_ARGS__)
void debug_print(const char *file, int line, const char *function, const char *message, ...) PRINTF_FORMAT(4, 5);
//************ ************//

void shutting_down(void);
void sigintHandler(int sig_num);

int get_free_id(void);
void set_free_by_int(int t);
void set_free_by_id(pthread_t t);
int get_int_from_id(pthread_t t);

int check_login(int login_id, char *password, int t);

int add_buy_order(int login_id, int item_code, int quantity, int unit_price);
int add_sell_order(int login_id, int item_code, int quantity, int unit_price);

int write_to_socket(int sock, char *text);
int read_from_socket(int sock, char *buffer);

#endif