#include "list_ops.h"

#include "helpers.h"
#include "main.h"

#include <string.h>

int list_insert(list_t *list, entry_t block) {
	int size = list_size(list);
	if(list_is_full(list))
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

int list_is_full(list_t *list) {
	return list_size(list) == MAX_RECORDS;
}

int list_get_min_price(list_t *list, entry_t *res, int *index) {
	if(list_is_empty(list)) {
		memset(res, 0, sizeof(entry_t));
		*index = -1;
		return 0;
	}
	int size = list_size(list);
	int price = 0;
	entry_t temp = list_get_entry(list, 0);
	entry_t temp2;
	int j;
	for(int i = 0; i < size - 1; i++) {
		temp2 = list_get_entry(list, i);
		if(temp.unit_price > temp2.unit_price) {
			temp = temp2;
			j = i;
			price = temp.unit_price;
		}
	}
	if(res != NULL)
		*res = temp;
	if(index != NULL)
		*index = j;
	return price;
}

int list_get_max_price(list_t *list, entry_t *res, int *index) {
	if(list_is_empty(list)) {
		memset(res, 0, sizeof(entry_t));
		*index = -1;
		return 0;
	}
	int size = list_size(list);
	int price = 0;
	entry_t temp = list_get_entry(list, 0);
	entry_t temp2;
	int j;
	for(int i = 0; i < size - 1; i++) {
		temp2 = list_get_entry(list, i);
		if(temp.unit_price < temp2.unit_price) {
			temp = temp2;
			j = i;
			price = temp.unit_price;
		}
	}
	if(res != NULL)
		*res = temp;
	if(index != NULL)
		*index = j;
	return price;
}

int entry_compare_price(entry_t a, entry_t b) {
	return a.unit_price - b.unit_price;
}

int entry_compare_quantity(entry_t a, entry_t b) {
	return a.quantity - b.quantity;
}

int ledger_insert(ledger_t *ledger, record_t block) {
	int size = ledger_size(ledger);
	if(ledger_is_full(ledger))
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

int ledger_is_full(ledger_t *ledger) {
	return ledger_size(ledger) == MAX_RECORDS;
}

record_t ledger_make_record(entry_t buyer, entry_t seller, int item, int is_buying) {
	record_t res;
	res.buyer = buyer.user;
	res.seller = seller.user;
	res.item = item;
	res.unit_price = is_buying ? seller.unit_price : buyer.unit_price;
	res.quantity = min(buyer.quantity, seller.quantity);
	return res;
}