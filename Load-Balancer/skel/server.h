/* Copyright 2023 <> */
#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>

typedef struct ll_node_t
{
	void *data; // This is cast to (info *)
	struct ll_node_t *next;
} ll_node_t;

typedef struct linked_list_t
{
	ll_node_t *head;
	unsigned int data_size;
	unsigned int size;
} linked_list_t;

typedef struct info info;
struct info {
	void *key;
	void *value;
};

typedef struct server_memory server_memory;
struct server_memory {
	linked_list_t **buckets; /* Array of singly linked lists. */

	/* Total number of nodes currently existing in all buckets. */
	unsigned int size;

	unsigned int hmax; /* Number of buckets. */

	/* (Pointer to) Function to calculate the hash value associated with keys. */
	unsigned int (*hash_function)(void *);

	/* (Pointer to) Function to compare two keys. */
	int (*compare_function)(void *, void *);

	/* (Pointer to) Function to free the memory occupied by key and value. */
	void (*key_val_free_function)(void *);
};

/** init_server_memory() - Initializes the memory for a new server struct.
 *                           Make sure to check what is returned by malloc using DIE.
 *                           Use the linked list implementation from the lab.
 *
 * Return: pointer to the allocated server_memory struct.
 */
server_memory *init_server_memory();

/** free_server_memory() - Free the memory used by the server.
 *                          Make sure to also free the pointer to the server struct.
 *                          You can use the server_remove() function for this.
 *
 * @arg1: Server to free
 */
void free_server_memory(server_memory *server);

/**
 * server_store() - Stores a key-value pair to the server.
 *
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 */
void server_store(server_memory *server, char *key, char *value);

/**
 * server_remove() - Removes a key-value pair from the server.
 *                    Make sure to free the memory of everything that is related to the entry removed.
 *
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 */
void server_remove(server_memory *server, char *key);

/**
 * server_retrieve() - Gets the value associated with the key.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 *
 * Return: String value associated with the key
 *         or NULL (in case the key does not exist).
 */
char *server_retrieve(server_memory *server, char *key);

#endif /* SERVER_H_ */
