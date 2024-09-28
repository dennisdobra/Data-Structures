/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"

linked_list_t *ll_create(unsigned int data_size)
{
	linked_list_t *ll;

	ll = malloc(sizeof(*ll));

	ll->head = NULL;
	ll->data_size = data_size;
	ll->size = 0;

	return ll;
}

void ll_add_nth_node(linked_list_t *list, unsigned int n, const void *new_data)
{
	ll_node_t *prev, *curr;
	ll_node_t *new_node;

	if (!list) {
		return;
	}

	if (n > list->size) {
		n = list->size;
	}

	curr = list->head;
	prev = NULL;
	while (n > 0) {
		prev = curr;
		curr = curr->next;
		--n;
	}

	new_node = malloc(sizeof(*new_node));
	new_node->data = malloc(list->data_size);
	memcpy(new_node->data, new_data, list->data_size);

	new_node->next = curr;
	if (prev == NULL) {
		list->head = new_node;
	}
	else {
		prev->next = new_node;
	}

	list->size++;
}

ll_node_t *ll_remove_nth_node(linked_list_t *list, unsigned int n)
{
	ll_node_t *prev, *curr;

	if (!list || !list->head) {
		return NULL;
	}

	if (n > list->size - 1) {
		n = list->size - 1;
	}

	curr = list->head;
	prev = NULL;
	while (n > 0) {
		prev = curr;
		curr = curr->next;
		--n;
	}

	if (prev == NULL) {
		/* Adica n == 0. */
		list->head = curr->next;
	}
	else {
		prev->next = curr->next;
	}

	list->size--;

	return curr;
}

unsigned int ll_get_size(linked_list_t *list)
{
	if (!list) {
		return -1;
	}

	return list->size;
}

void ll_free(linked_list_t **pp_list)
{
	ll_node_t *currNode;

	if (!pp_list || !*pp_list) {
		return;
	}

	while (ll_get_size(*pp_list) > 0) {
		currNode = ll_remove_nth_node(*pp_list, 0);
		free(((info *)currNode->data)->key);
		free(((info *)currNode->data)->value);
		free(currNode->data);
		currNode->data = NULL;
		free(currNode);
		currNode = NULL;
	}

	free(*pp_list);
	*pp_list = NULL;
}

void ll_print_int(linked_list_t *list)
{
	ll_node_t *curr;

	if (!list) {
		return;
	}

	curr = list->head;
	while (curr != NULL) {
		printf("%d ", *((int *)curr->data));
		curr = curr->next;
	}

	printf("\n");
}

void ll_print_string(linked_list_t *list)
{
	ll_node_t *curr;

	if (!list) {
		return;
	}

	curr = list->head;
	while (curr != NULL) {
		printf("%s ", (char *)curr->data);
		curr = curr->next;
	}

	printf("\n");
}

void key_val_free_function(void *data)
{
	free(((info *)data)->key);
	free(((info *)data)->value);
	free(data);
}

// hash function for a string
unsigned int hash_function_string(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

// function that allocates memory for a server
server_memory *init_server_memory()
{
	int hmax = 50;
	server_memory *server;
	server = malloc(sizeof(server_memory));
	server->hmax = hmax;
	server->size = 0;
	server->compare_function = compare_function_strings;
	server->hash_function = hash_function_string;
	server->key_val_free_function = key_val_free_function;
	server->buckets = malloc(hmax * sizeof(linked_list_t));
	for (int i = 0; i < hmax; i++)     {
		server->buckets[i] = ll_create(sizeof(info));
	}
	return server;
}

void server_store(server_memory *server, char *key, char *value) {
	int hash = server->hash_function(key);
	char *node = server_retrieve(server, key);

	if (node == NULL){
		int index = hash % server->hmax;
		info *data = malloc(sizeof(info));
		data->key = malloc(128 * sizeof(char));
		memcpy(data->key, key, sizeof(key));
		data->value = malloc(65536 * sizeof(char));
		memcpy(data->value, value, sizeof(value));
		ll_add_nth_node(server->buckets[index], server->buckets[index]->size, data);
		free(data);
		server->size++;
	}	
	else {
		memcpy(node, value, sizeof(value));
	}
}

char *server_retrieve(server_memory *server, char *key) {
	int hash = server->hash_function(key);
	int index = hash % server->hmax;
	ll_node_t *current = server->buckets[index]->head;
	while (current != NULL)     {
		if (server->compare_function(((info *)current->data)->key, key) == 0) {
			return ((info *)current->data)->value;
		}
		current = current->next;
	}
	return NULL;
}

void server_remove(server_memory *server, char *key) {
	int hash = server->hash_function(key);
	int index = hash % server->hmax;
	ll_node_t *current = server->buckets[index]->head;
	int n = 0;
	while (current != NULL) {
		if (server->compare_function(((info *)current->data)->key, key) == 0) {
			break;
		}
		current = current->next;
		n++;
	}
	current = ll_remove_nth_node(server->buckets[index], n);
	server->size--;
	key_val_free_function(current->data);
	free(current);
}

void free_server_memory(server_memory *server) {
	for(int i = 0; i < server->hmax; i++)
    {
        ll_free(&server->buckets[i]);
    }
    free(server->buckets);
    free(server);
}
