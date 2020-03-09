#ifndef _LIST_OPS_
#define _LIST_OPS_

#include "helpers.h"
#include "list_ops.h"
#include "main.h"

int list_insert(list_t *list, entry_t block);
int list_remove(list_t *list, int index);
int list_get_index(list_t *list, entry_t block);
entry_t list_get_entry(list_t *list, int t);
int list_is_empty(list_t *list);
int list_size(list_t *list);
int list_is_full(list_t *list);
int list_get_min_price(list_t *list, entry_t *res, int *index);
int list_get_max_price(list_t *list, entry_t *res, int *index);
int entry_compare_price(entry_t a, entry_t b);
int entry_compare_quantity(entry_t a, entry_t b);

int ledger_insert(ledger_t *ledger, record_t block);
// no need for ledger_remove
int ledger_get_index(ledger_t *ledger, record_t block);
record_t ledger_get_record(ledger_t *ledger, int t);
int ledger_is_empty(ledger_t *ledger);
int ledger_size(ledger_t *ledger);
int ledger_is_full(ledger_t *ledger);
record_t ledger_make_record(entry_t buyer, entry_t seller, int item, int is_buying);

#endif