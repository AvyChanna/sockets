#ifndef _LIST_OPS_
#define _LIST_OPS_

#include "list_ops.h"
#include "main.h"

int list_insert(list_t *list, entry_t block);
int list_remove(list_t *list, int index);
int list_get_index(list_t *list, entry_t block);
entry_t list_get_entry(list_t *list, int t);
int list_is_empty(list_t *list);
int list_size(list_t *list);
int list_find_min(list_t *list, entry_t *res);
int list_find_max(list_t *list, entry_t *res);

int ledger_insert(ledger_t *ledger, record_t block);
// no need for ledger_remove
int ledger_get_index(ledger_t *ledger, record_t block);
record_t ledger_get_record(ledger_t *ledger, int t);
int ledger_is_empty(ledger_t *ledger);
int ledger_size(ledger_t *ledger);

#endif