#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dll_node_t dll_node_t;
struct dll_node_t
{
	void *data;
	dll_node_t *prev, *next;
};

typedef struct list_t list_t;
struct list_t
{
	dll_node_t *head;
	unsigned int data_size;
	unsigned int size;
};

list_t *dll_create(unsigned int data_size);

dll_node_t *dll_get_nth_node(list_t *list, unsigned int n);

dll_node_t *create_node(void *new_data, int data_size);

void dll_add_nth_node(list_t *list, unsigned int n, const void *new_data);

dll_node_t *dll_remove_nth_node(list_t *list, unsigned int n);

unsigned int dll_get_size(list_t *list);

void dll_free(list_t *pp_list);

void dll_print_int_list(list_t *list);

void dll_print_string_list(list_t *list);

typedef struct
{
	uint64_t start_address;
	size_t size;
	void *miniblock_list;
} block_t;

typedef struct
{
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
} miniblock_t;

typedef struct
{
	uint64_t arena_size;
	list_t *alloc_list;
} arena_t;

/* declared functions */
arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
void free_block(arena_t *arena, const uint64_t address);

void read(arena_t *arena, uint64_t address, uint64_t size);
void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, int8_t *permission);
