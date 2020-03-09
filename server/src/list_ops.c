#include "list_ops.h"

#include "main.h"

#include <string.h>
int list_insert(list_t *list, entry_t block) {
	int size = list_size(list);
	if(size >= MAX_RECORDS)
		return 0;
	list->entry[size].user = block.user;
	list->entry[size].quantity = block.quantity;
	list->entry[size].unit_price = block.unit_price;
	size++;
	return 1;
}
int list_remove(list_t *list, int index) {
	int size = list_size(list);
	if(index >= size || index < 0)
		return 0;
	size--;
	for(int i = index; i < size; i++)
		list->entry[i] = list->entry[i + 1];
	return 1;
}
int list_get_index(list_t *list, entry_t block) {
	int size = list_size(list);
	entry_t comp;
	for(int i = 0; i < size; i++) {
		comp = list->entry[i];
		if(comp.user == block.user && comp.quantity == block.quantity && comp.unit_price == block.unit_price)
			return i;
	}
	return -1;
}
entry_t list_get_entry(list_t *list, int t) {
	return list->entry[t];
}
int list_is_empty(list_t *list) {
	return list_size(list) == 0;
}

int list_size(list_t *list) {
	return list->size;
}
int list_find_min(list_t *list, entry_t *res) {
	if(list_is_empty(list)){
		memset(res, 0, sizeof(entry_t));
		return 0;
	}
	int size = list_size(list);
	*res = list_get_entry(list, 0);
	for(int i = 0; i < size - 1; i++) {
		if(res->unit_price > list_get_entry(list, i).unit_price)
			*res = list_get_entry(list, i);
	}
	return 1;
}
int list_find_max(list_t *list, entry_t *res) {
	if(list_is_empty(list)) {
		memset(res, 0, sizeof(entry_t));
		return 0;
	}
	int size = list_size(list);
	*res = list_get_entry(list, 0);
	for(int i = 0; i < size - 1; i++) {
		if(res->unit_price < list_get_entry(list, i).unit_price)
			*res = list_get_entry(list, i);
	}
	return 1;
}

int ledger_insert(ledger_t *ledger, record_t block) {
	int size = ledger_size(ledger);
	if(size >= MAX_RECORDS)
		return 0;
	ledger->record[size].buyer = block.buyer;
	ledger->record[size].seller = block.seller;
	ledger->record[size].item = block.item;
	ledger->record[size].quantity = block.quantity;
	ledger->record[size].unit_price = block.unit_price;
	size++;
	return 1;
}
int ledger_get_index(ledger_t *ledger, record_t block) {
	int size = ledger_size(ledger);
	record_t comp;
	for(int i = 0; i < size; i++) {
		comp = ledger->record[i];
		if(comp.buyer == block.buyer && comp.seller == block.seller && comp.item == block.item &&
		   comp.quantity == block.quantity && comp.unit_price == block.unit_price)
			return i;
	}
	return -1;
}
record_t ledger_get_record(ledger_t *ledger, int t) {
	return ledger->record[t];
}
int ledger_is_empty(ledger_t *ledger) {
	return ledger_size(ledger) == 0;
}
int ledger_size(ledger_t *ledger) {
	return ledger->size;
}