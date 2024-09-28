#include "vma.h"

/* FUNCTIONS FOR LIST OPERATIONS */
list_t *dll_create(unsigned int data_size)
{
	list_t *list = malloc(sizeof(list_t));
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;
	return list;
}

dll_node_t *dll_get_nth_node(list_t *list, unsigned int n)
{
	dll_node_t *curr;
	if (list->size == 0)
		return 0;

	curr = list->head;
	for (unsigned int i = 0; i < n % list->size; i++)
		curr = curr->next;

	return curr;
}

dll_node_t *create_node(void *new_data, int data_size)
{
	dll_node_t *new_node = malloc(sizeof(dll_node_t));
	new_node->data = malloc(data_size);
	memcpy(new_node->data, new_data, data_size);
	return new_node;
}

void dll_add_nth_node(list_t *list, unsigned int n, const void *new_data)
{
	dll_node_t *new_node = create_node((void *)new_data, list->data_size);

	if (list->size == 0) {
		list->head = new_node;
		new_node->next = new_node;
		new_node->prev = new_node;
		list->size++;
		return;
	}

	if (n == 0) {
		new_node->next = list->head;
		new_node->prev = list->head->prev;

		list->head->prev->next = new_node;
		list->head->prev = new_node;

		list->head = new_node;
		list->size++;
		return;
	}

	if (n > list->size) {
		n = list->size;
	}

	dll_node_t *curr = list->head;
	for (unsigned int i = 0; i < n - 1; i++) {
		curr = curr->next;
	}

	new_node->next = curr->next;
	new_node->prev = curr;
	curr->next = new_node;
	new_node->next->prev = new_node;
	list->size++;
}

dll_node_t *dll_remove_nth_node(list_t *list, unsigned int n)
{
	if (list->size == 0) {
		return 0;
	}

	if (n == 0) {
		dll_node_t *aux = list->head;
		list->head = list->head->next;
		list->head->prev = list->head->prev->prev;
		list->head->prev = list->head;
		list->size--;
		return aux;
	}

	if (n >= list->size - 1) {
		n = list->size - 1;
	}

	dll_node_t *curr = list->head;
	for (unsigned int i = 0; i < n - 1; i++) {
		curr = curr->next;
	}
	dll_node_t *aux = curr->next;

	curr->next->next->prev = curr;
	curr->next = curr->next->next;

	list->size--;
	return aux;
}

unsigned int dll_get_size(list_t *list)
{
	return list->size;
}

void dll_free(list_t *pp_list)
{
	if (pp_list->size > 0) {
		dll_node_t *curr = pp_list->head;
		dll_node_t *aux;
		for (unsigned int i = 0; i < pp_list->size; i++) {
			aux = curr->next;
			free(curr->data);
			free(curr);
			curr = aux;
		}
	}
	free(pp_list);
}

void dll_print_int_list(list_t *list)
{
	dll_node_t *curr = list->head;

	for (unsigned int i = 0; i < list->size; i++) {
		printf("%d ", *(int *)curr->data);
		curr = curr->next;
	}
	printf("\n");
}

void dll_print_string_list(list_t *list)
{
	dll_node_t *curr = list->head->prev;

	for (unsigned int i = list->size - 1; i > 0; i--) {
		printf("%s ", (char *)curr->data);
		curr = curr->prev;
	}
	printf("\n");
}

/* FUNCTIONS FOR ARENA */
arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena = malloc(sizeof(arena_t));        // allocate space for the entire arena
	arena->arena_size = size;                        // store the size of the arena
	arena->alloc_list = dll_create(sizeof(block_t)); // allocate space for the list of blocks
	arena->alloc_list->size = 0;
	return arena;
}

void dealloc_arena(arena_t *arena)
{
	if (arena->alloc_list->size != 0) {
		dll_node_t *curr_block = arena->alloc_list->head;
		for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
			dll_node_t *curr_mblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
			for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {
				if (((miniblock_t *)curr_mblock->data)->rw_buffer != NULL)
					free(((miniblock_t *)curr_mblock->data)->rw_buffer); // free the data in the current miniblock
				curr_mblock = curr_mblock->next;
			}
			dll_free((list_t *)(((block_t *)curr_block->data)->miniblock_list)); // free the miniblock list in the current block
			curr_block = curr_block->next;
		}
	}

	dll_free(arena->alloc_list); // free the block list in the arena
	free(arena);                 // free the arena
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	// address exceeds the size of the arena
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
	}

	// if address + size > arena_size
	else if (address + size > arena->arena_size) // doesn't need to be >=
	{
		printf("The end address is past the size of the arena\n");
		return;
	}
	else {
		// case where the arena is empty, with no blocks
		if (arena->alloc_list->size == 0) {
			block_t *new_block = malloc(sizeof(block_t));
			new_block->start_address = address;
			new_block->size = size;

			new_block->miniblock_list = dll_create(sizeof(miniblock_t)); // allocate space for the list of miniblocks
			miniblock_t *new_miniblock = malloc(sizeof(miniblock_t));
			new_miniblock->start_address = address;
			new_miniblock->size = size;
			new_miniblock->rw_buffer = NULL;
			new_miniblock->perm = 6;

			dll_add_nth_node(new_block->miniblock_list, 0, new_miniblock); // added miniblock to the list of miniblocks (automatically becomes head)
			dll_add_nth_node(arena->alloc_list, 0, new_block);             // added block to the list of blocks (automatically becomes head)

			free(new_miniblock);
			free(new_block);
			return;
		}
		// if an isolated block is added which would become head
		else if (address < ((block_t *)arena->alloc_list->head->data)->start_address &&
			(address + size < ((block_t *)arena->alloc_list->head->data)->start_address)) {
			// check if the input address is smaller than the address of the first block in the arena
			// if this case is true, it assumes the head already exists
			block_t *new_block = malloc(sizeof(block_t));
			new_block->start_address = address;
			new_block->size = size;

			new_block->miniblock_list = dll_create(sizeof(miniblock_t)); // allocate space for the list of miniblocks
			miniblock_t *new_miniblock = malloc(sizeof(miniblock_t));
			new_miniblock->start_address = address;
			new_miniblock->size = size;
			new_miniblock->rw_buffer = NULL;
			new_miniblock->perm = 6;

			dll_add_nth_node(new_block->miniblock_list, 0, new_miniblock); // added miniblock to the list of miniblocks
			dll_add_nth_node(arena->alloc_list, 0, new_block);             // added block to the list of blocks

			free(new_miniblock);
			free(new_block);
			return;
		}
		// there is at least one block
		else {
			dll_node_t *curr_block = arena->alloc_list->head;

			for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
				uint64_t curr_block_start = ((block_t *)curr_block->data)->start_address;
				uint64_t curr_block_stop = curr_block_start + ((block_t *)curr_block->data)->size;

				if (address < curr_block_start && address + size > curr_block_start) {
					printf("This zone was already allocated.\n");
					return;
				}

				// means the address is within a block
				else if (address >= curr_block_start && address < curr_block_stop) {
					printf("This zone was already allocated.\n");
					return;
				}
				// means the address is valid
				else {
					// case where an isolated block is added anywhere
					if (((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size < address &&
						(curr_block->next == curr_block || address + size < ((block_t *)curr_block->next->data)->start_address || curr_block->next == arena->alloc_list->head)) {

						block_t *new_block = malloc(sizeof(block_t));
						new_block->start_address = address;
						new_block->size = size;

						new_block->miniblock_list = dll_create(sizeof(miniblock_t)); // allocate space for the list of miniblocks
						miniblock_t *new_miniblock = malloc(sizeof(miniblock_t));
						new_miniblock->start_address = address;
						new_miniblock->size = size;
						new_miniblock->rw_buffer = NULL;
						new_miniblock->perm = 6;

						dll_add_nth_node(new_block->miniblock_list, 0, new_miniblock); // added miniblock to the list of miniblocks
						dll_add_nth_node(arena->alloc_list, i + 1, new_block);         // added block to the list of blocks

						free(new_miniblock);
						free(new_block);
						return;
					}

					// case where the added block is free on the right, but joins with the left
					else if (((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size == address &&
						(curr_block->next == curr_block || address + size < ((block_t *)curr_block->next->data)->start_address || curr_block->next == arena->alloc_list->head)) {

						((block_t *)curr_block->data)->size += size; // increase the size of the initial block
						miniblock_t *new_miniblock = malloc(sizeof(miniblock_t));
						// set start_address, size, and perm for the new miniblock
						new_miniblock->start_address = address;
						new_miniblock->size = size;
						new_miniblock->rw_buffer = NULL;
						new_miniblock->perm = 6;

						dll_add_nth_node((list_t *)(((block_t *)curr_block->data)->miniblock_list), ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size, new_miniblock);

						free(new_miniblock);
						return;
					}

					// case where the added block is free on the left, but joins with the right
					else if (address + size == ((block_t *)curr_block->data)->start_address && (address > ((block_t *)curr_block->prev->data)->start_address + ((block_t *)curr_block->prev->data)->size || curr_block == arena->alloc_list->head)) {

						miniblock_t *new_miniblock = malloc(sizeof(miniblock_t));
						((block_t *)curr_block->data)->size += size;
						// set start_address, size, and perm for the new miniblock
						new_miniblock->start_address = address;
						new_miniblock->size = size;
						new_miniblock->rw_buffer = NULL;
						new_miniblock->perm = 6;
						// modify the start address of the current block
						((block_t *)curr_block->data)->start_address = address;
						dll_add_nth_node(((list_t *)((block_t *)curr_block->data)->miniblock_list), 0, new_miniblock);

						free(new_miniblock);
						return;
					}

					// case where the added block joins both left and right
					else if (((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size == address &&
						address + size == ((block_t *)curr_block->next->data)->start_address) {

						// create a copy of curr_block->next
						dll_node_t *copy = curr_block->next;

						// update the size of the current block
						((block_t *)curr_block->data)->size += size + ((block_t *)curr_block->next->data)->size;
						miniblock_t *new_miniblock = malloc(sizeof(miniblock_t));

						new_miniblock->start_address = address;
						new_miniblock->size = size;
						new_miniblock->rw_buffer = NULL;
						new_miniblock->perm = 6;

						dll_add_nth_node(((list_t *)((block_t *)curr_block->data)->miniblock_list), ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size, new_miniblock);
						// adjust the links
						curr_block->next = curr_block->next->next;
						curr_block->next->prev = curr_block;

						arena->alloc_list->size--;
						((list_t *)((block_t *)curr_block->data)->miniblock_list)->size += ((list_t *)((block_t *)copy->data)->miniblock_list)->size;
						// adjust the links for the block on the right (curr_block->next)
						dll_node_t *aux = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head->prev;
						((list_t *)((block_t *)curr_block->data)->miniblock_list)->head->prev->next = ((list_t *)((block_t *)copy->data)->miniblock_list)->head;
						((list_t *)((block_t *)curr_block->data)->miniblock_list)->head->prev = ((list_t *)((block_t *)copy->data)->miniblock_list)->head->prev;
						((list_t *)((block_t *)copy->data)->miniblock_list)->head->prev->next = aux;
						((list_t *)((block_t *)copy->data)->miniblock_list)->head->prev = aux;

						// free memory
						free(copy);
						free(new_miniblock);
						return;
					}
				}
				curr_block = curr_block->next;
			}
		}
	}
}

void free_block(arena_t *arena, const uint64_t address)
{
	int valid_adr = 0; // if the given address is the start of a miniblock, this variable becomes 1
	int ok = 0; // if the block containing the given address is found, ok will become 1
	dll_node_t *curr_block = arena->alloc_list->head;

	if (address >= arena->arena_size) {
		printf("Invalid address for free.\n");
		return;
	}
	// if it's before the first block
	if (arena->alloc_list->size == 0 || address < ((block_t *)arena->alloc_list->head->data)->start_address) {
		printf("Invalid address for free.\n");
		return;
	}
	// if it's between the start address of the last block and the end of the arena
	else if (address > ((miniblock_t *)((list_t *)((block_t *)arena->alloc_list->head->prev->data)->miniblock_list)->head->prev->data)->start_address) {
		printf("Invalid address for free.\n");
		return;
	}

	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		dll_node_t *curr_mblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;

		uint64_t start = ((block_t *)curr_block->data)->start_address;
		uint64_t stop = start + ((block_t *)curr_block->data)->size;
		// select the block where the given address is located
		if (start <= address && address < stop) {
			ok = 1;
			// check if the address is the start of any miniblock
			for (unsigned int k = 0; k < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; k++) {
				uint64_t begin = ((miniblock_t *)curr_mblock->data)->start_address;
				if (address == begin) {
					valid_adr = 1;
				}
				curr_mblock = curr_mblock->next;
			}
			if (valid_adr == 0) {
				printf("Invalid address for free.\n");
				return;
			}

			curr_mblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;

			// check if the size of the block is equal to the size of the miniblock (case where there is only one miniblock)
			if (((block_t *)curr_block->data)->size == ((miniblock_t *)curr_mblock->data)->size) {

				free(((miniblock_t *)curr_mblock->data)->rw_buffer); // free the content of the miniblock
				free(curr_mblock->data);                             // free the miniblock structure
				free(curr_mblock);                                   // free the node
				free(((block_t *)curr_block->data)->miniblock_list);  // delete the miniblock list

				free(curr_block->data);
				// if the block is the head of the arena =>
				if (curr_block == arena->alloc_list->head) {
					arena->alloc_list->head = curr_block->next; // => change the head
				}
				curr_block->next->prev = curr_block->prev;
				curr_block->prev->next = curr_block->next;

				arena->alloc_list->size--; // decrease the number of blocks in the arena

				free(curr_block); // delete the current block
			}
			// if the miniblock is at the beginning of the block
			else if (address == ((block_t *)curr_block->data)->start_address) {
				((block_t *)curr_block->data)->start_address = ((miniblock_t *)curr_mblock->next->data)->start_address;               // change the start address of the block to the second miniblock in the block
				((block_t *)curr_block->data)->size = ((block_t *)curr_block->data)->size - ((miniblock_t *)curr_mblock->data)->size; // change the size of the block

				curr_mblock->next->prev = curr_mblock->prev; // change the links between miniblocks
				curr_mblock->prev->next = curr_mblock->next;
				((list_t *)((block_t *)curr_block->data)->miniblock_list)->head = curr_mblock->next;

				((list_t *)((block_t *)curr_block->data)->miniblock_list)->size--; // decrease the number of miniblocks in the block

				free(((miniblock_t *)curr_mblock->data)->rw_buffer);
				free(curr_mblock->data);
				free(curr_mblock);
			}
			// if it's at the end <=> given address = start address of the last miniblock
			else if (address == ((miniblock_t *)curr_mblock->prev->data)->start_address) {
				curr_mblock = curr_mblock->prev; // so that it's the last miniblock in the block

				((block_t *)curr_block->data)->size -= ((miniblock_t *)curr_mblock->data)->size;
				curr_mblock->prev->next = curr_mblock->next; // the second-last miniblock becomes the head;
				curr_mblock->next->prev = curr_mblock->prev; // the head goes to the second-last

				((list_t *)((block_t *)curr_block->data)->miniblock_list)->size--; // decrease the number of miniblocks in the block

				free(((miniblock_t *)curr_mblock->data)->rw_buffer);
				free(curr_mblock->data);
				free(curr_mblock);
			}
			// if it's in the middle of the block
			else if (address > ((block_t *)curr_block->data)->start_address &&
				address < ((miniblock_t *)curr_mblock->prev->data)->start_address) {
				int cont = 0; // counts miniblocks in the left block
				// the current miniblock is the head of the selected block - selected earlier
				for (unsigned int i = 0; i < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; i++) {
					if (address == ((miniblock_t *)curr_mblock->data)->start_address) {
						// found the start of the miniblock to delete

						// remember the size of the large block
						// remember the number of miniblocks in the large block
						// remember the previous miniblock of the first miniblock in the current block
						unsigned int curr_block_size = ((block_t *)curr_block->data)->size;
						unsigned int nr_miniblocks_initial = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; // 10
						dll_node_t *last_miniblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head->prev;

						// modify the size of the first block
						// remember how many miniblocks the left block will have
						((block_t *)curr_block->data)->size = ((miniblock_t *)curr_mblock->next->data)->start_address - ((block_t *)curr_block->data)->start_address - ((miniblock_t *)curr_mblock->data)->size;
						((list_t *)((block_t *)curr_block->data)->miniblock_list)->size = cont;

						// take an aux which will be the first miniblock in the block
						// link the miniblocks in the first block
						dll_node_t *aux1 = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
						curr_mblock->prev->next = aux1;
						aux1->prev = curr_mblock->prev;

						// allocate a node to create the right block
						dll_node_t *new_node = malloc(sizeof(dll_node_t));
						new_node->data = malloc(sizeof(block_t));
						((block_t *)new_node->data)->miniblock_list = dll_create(sizeof(miniblock_t)); // create the large list

						// set the start address of the second block = start address of the next miniblock
						// set the size of the new block
						// set the size of the new miniblock list
						// set the head in new_node
						// make the links
						((block_t *)new_node->data)->start_address = ((block_t *)curr_mblock->next->data)->start_address;
						((block_t *)new_node->data)->size = curr_block_size - ((miniblock_t *)curr_mblock->data)->size - ((block_t *)curr_block->data)->size;
						((list_t *)((block_t *)new_node->data)->miniblock_list)->size = nr_miniblocks_initial - cont - 1; // 10 - 5 - 1
						((list_t *)((block_t *)new_node->data)->miniblock_list)->head = curr_mblock->next;
						((list_t *)((block_t *)new_node->data)->miniblock_list)->head->prev = last_miniblock;
						last_miniblock->next = ((list_t *)((block_t *)new_node->data)->miniblock_list)->head;

						free(((miniblock_t *)curr_mblock->data)->rw_buffer);
						free(curr_mblock->data);
						free(curr_mblock);
					}
					cont++;
				}
			}
		}
		if (ok == 1) // if we found the correct block, we don't need to check the rest
			break;
		curr_block = curr_block->next;
	}
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	uint64_t copie_size = size; // size is a read-only parameter => make a copy
	dll_node_t *curr_block = arena->alloc_list->head;

	// the given address does not exist
	if (arena->alloc_list->size == 0) {
		printf("Invalid address for read.\n");
	}
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		if (address >= arena->arena_size ||
			(address < ((block_t *)arena->alloc_list->head->data)->start_address && ((block_t *)arena->alloc_list->head->data)->start_address != 0) ||
			(address >= ((block_t *)curr_block->prev->data)->start_address + ((block_t *)curr_block->prev->data)->size && address < ((block_t *)curr_block->data)->start_address) ||
			(address >= ((block_t *)arena->alloc_list->head->prev->data)->start_address + ((block_t *)arena->alloc_list->head->prev->data)->size)) {
			printf("Invalid address for read.\n");
			return;
		}
		curr_block = curr_block->next;
	}
	// check if the block has read permissions
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		dll_node_t *curr_mblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
		for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {
			uint64_t mblock_start = ((miniblock_t *)curr_mblock->data)->start_address;
			uint64_t mblock_stop = mblock_start + ((miniblock_t *)curr_mblock->data)->size;
			if (address >= mblock_start && address < mblock_stop) {
				while (j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size) {
					if (((miniblock_t *)curr_mblock->data)->perm < 4) {
						printf("Invalid permissions for read.\n");
						return;
					}
					curr_mblock = curr_mblock->next;
					j++;
				}
			}
			curr_mblock = curr_mblock->next;
		}
		curr_block = curr_block->next;
	}

	// the address exists
	curr_block = arena->alloc_list->head;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		// remember the stop address of each block
		uint64_t stop = ((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size;

		// if it starts in the current block and exceeds the current block
		if (address + size > stop && (address >= ((block_t *)curr_block->data)->start_address && address < stop)) {
			// found the correct block where the address is located
			int nr_characters = stop - address;
			printf("Warning: size was bigger than the block size. Reading %d characters.\n", nr_characters);

			// now look for the miniblock in the block where the address is located
			dll_node_t *curr_miniblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
			for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {
				uint64_t miniblock_start = ((miniblock_t *)curr_miniblock->data)->start_address;
				uint64_t miniblock_stop = miniblock_start + ((miniblock_t *)curr_miniblock->data)->size;

				int k = 0;
				if (address >= miniblock_start && address < miniblock_stop) {
					// found the miniblock from which to start
					uint64_t contor = address;
					while (copie_size != 0 && contor < stop) // as long as there is data left and we haven't exceeded the block's end
					{
						miniblock_stop = ((miniblock_t *)curr_miniblock->data)->start_address + ((miniblock_t *)curr_miniblock->data)->size;

						while (contor < miniblock_stop && copie_size != 0) {
							printf("%c", ((int8_t *)((miniblock_t *)curr_miniblock->data)->rw_buffer)[contor - ((miniblock_t *)curr_miniblock->data)->start_address]);
							k++;
							copie_size--;
							contor++;
							if (copie_size == 0 || contor == ((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size) {
								printf("\n");
								return;
							}
						}
						curr_miniblock = curr_miniblock->next; // move to the next miniblock to write in it
					}
				}
				curr_miniblock = curr_miniblock->next; // still looking for the miniblock to start from
			}
		}
		// if it finishes reading within the same block
		else if (address >= ((block_t *)curr_block->data)->start_address && address < stop) {
			// now look for the miniblock in the block where the address is located
			dll_node_t *curr_miniblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
			for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {
				uint64_t miniblock_start = ((miniblock_t *)curr_miniblock->data)->start_address;
				uint64_t miniblock_stop = miniblock_start + ((miniblock_t *)curr_miniblock->data)->size;

				int k = 0;
				if (address >= miniblock_start && address < miniblock_stop) {
					// found the miniblock from which to start
					uint64_t contor = address;
					while (copie_size != 0 && contor < stop) // as long as there is data left and we haven't exceeded the block's end
					{
						miniblock_stop = ((miniblock_t *)curr_miniblock->data)->start_address + ((miniblock_t *)curr_miniblock->data)->size;

						while (contor < miniblock_stop && copie_size != 0) {
							printf("%c", ((int8_t *)((miniblock_t *)curr_miniblock->data)->rw_buffer)[contor - ((miniblock_t *)curr_miniblock->data)->start_address]);
							k++;
							copie_size--;
							contor++;
							if (copie_size == 0 || contor == ((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size) {
								printf("\n");
								return;
							}
						}
						curr_miniblock = curr_miniblock->next; // move to the next miniblock to write in it
					}
					printf("\n");
				}
				curr_miniblock = curr_miniblock->next; // still looking for the miniblock to start from
			}
		}
		curr_block = curr_block->next;
	}
}

void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	uint64_t copie_size = size; // size is a read-only parameter => I make a copy of it
	dll_node_t *curr_block = arena->alloc_list->head;

	// the given address does not exist
	if (arena->alloc_list->size == 0) {
		printf("Invalid address for write.\n");
	}
	// check if the block has write permissions
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		dll_node_t *curr_mblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
		for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {
			uint64_t mblock_start = ((miniblock_t *)curr_mblock->data)->start_address;
			uint64_t mblock_stop = mblock_start + ((miniblock_t *)curr_mblock->data)->size;
			if (address >= mblock_start && address < mblock_stop) {
				while (j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size) {
					if (((miniblock_t *)curr_mblock->data)->perm != 2 &&
						((miniblock_t *)curr_mblock->data)->perm != 3 &&
						((miniblock_t *)curr_mblock->data)->perm != 6 &&
						((miniblock_t *)curr_mblock->data)->perm != 7) {
						printf("Invalid permissions for write.\n");
						return;
					}
					j++;
					curr_mblock = curr_mblock->next;
				}
			}
			curr_mblock = curr_mblock->next;
		}
		curr_block = curr_block->next;
	}

	curr_block = arena->alloc_list->head;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		if (address >= arena->arena_size ||
			(address < ((block_t *)arena->alloc_list->head->data)->start_address) ||
			(address >= ((block_t *)curr_block->prev->data)->start_address + ((block_t *)curr_block->prev->data)->size && address < ((block_t *)curr_block->data)->start_address) ||
			(address >= ((block_t *)arena->alloc_list->head->prev->data)->start_address + ((block_t *)arena->alloc_list->head->prev->data)->size && address < arena->arena_size)) {
			printf("Invalid address for write.\n");
			return;
		}
		curr_block = curr_block->next;
	}

	// the address exists
	curr_block = arena->alloc_list->head;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		// keep track of the end of each block
		uint64_t stop = ((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size;
		// if it starts in the current block and exceeds the current block
		if (address + size > stop && (address >= ((block_t *)curr_block->data)->start_address && address < stop)) {
			// found the correct block where the address is located
			int nr_characters = stop - address;
			printf("Warning: size was bigger than the block size. Writing %d characters.\n", nr_characters);

			// now I search for the miniblock in the block where the address is located
			dll_node_t *curr_miniblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
			for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {
				uint64_t miniblock_start = ((miniblock_t *)curr_miniblock->data)->start_address;
				uint64_t miniblock_stop = miniblock_start + ((miniblock_t *)curr_miniblock->data)->size;

				int k = 0;
				if (address >= miniblock_start && address < miniblock_stop) {
					// found the miniblock where I start
					uint64_t contor = address;
					while (copie_size != 0 && contor < stop) // while I still have data and I don't go beyond the block
					{
						if (((miniblock_t *)curr_miniblock->data)->rw_buffer == NULL)
							((miniblock_t *)curr_miniblock->data)->rw_buffer = malloc(((miniblock_t *)curr_miniblock->data)->size);
						miniblock_stop = ((miniblock_t *)curr_miniblock->data)->start_address + ((miniblock_t *)curr_miniblock->data)->size;

						while (contor < miniblock_stop && copie_size != 0) {
							((int8_t *)((miniblock_t *)curr_miniblock->data)->rw_buffer)[contor - ((miniblock_t *)curr_miniblock->data)->start_address] = data[k];
							k++;
							copie_size--;
							contor++;
							if (copie_size == 0 || contor == ((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size) {
								return;
							}
						}
						curr_miniblock = curr_miniblock->next; // move to the next miniblock to write in it
					}
				}
				curr_miniblock = curr_miniblock->next; // still searching for the miniblock where I start
			}
		}
		// otherwise, if it finishes writing in the same block
		else if (address >= ((block_t *)curr_block->data)->start_address && address < stop) {
			// now I search for the miniblock in the block where the address is located
			dll_node_t *curr_miniblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
			for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {
				uint64_t miniblock_start = ((miniblock_t *)curr_miniblock->data)->start_address;
				uint64_t miniblock_stop = miniblock_start + ((miniblock_t *)curr_miniblock->data)->size;

				int k = 0;
				if (address >= miniblock_start && address < miniblock_stop) {
					// found the miniblock where I start
					uint64_t contor = address;
					while (copie_size != 0 && contor < stop) // while I still have data and I don't go beyond the block
					{
						miniblock_stop = ((miniblock_t *)curr_miniblock->data)->start_address + ((miniblock_t *)curr_miniblock->data)->size;
						if (((miniblock_t *)curr_miniblock->data)->rw_buffer == NULL)
							((miniblock_t *)curr_miniblock->data)->rw_buffer = malloc(((miniblock_t *)curr_miniblock->data)->size);

						while (contor < miniblock_stop && copie_size != 0) {
							((int8_t *)((miniblock_t *)curr_miniblock->data)->rw_buffer)[contor - ((miniblock_t *)curr_miniblock->data)->start_address] = data[k];
							k++;
							copie_size--;
							contor++;
							if (copie_size == 0 || contor == ((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size) {
								return;
							}
						}
						curr_miniblock = curr_miniblock->next; // move to the next miniblock to write in it
					}
				}
				curr_miniblock = curr_miniblock->next; // still searching for the miniblock where I start
			}
		}
		curr_block = curr_block->next;
	}
}

void pmap(const arena_t *arena)
{
    printf("Total memory: 0x%lX bytes\n", arena->arena_size);
    // Traverse all blocks to sum up the size
    uint64_t memory = 0;
    int total_miniblocks = 0;

    if (arena->alloc_list->size != 0) {
        dll_node_t *curr_block = arena->alloc_list->head;
        for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
            memory = memory + ((block_t *)curr_block->data)->size;
            total_miniblocks = total_miniblocks + ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size;
            curr_block = curr_block->next;
        }
    }

    uint64_t free_memory = arena->arena_size - memory;
    printf("Free memory: 0x%lX bytes\n", free_memory);
    printf("Number of allocated blocks: %d\n", arena->alloc_list->size);
    printf("Number of allocated miniblocks: %d\n", total_miniblocks);
    if (total_miniblocks != 0) {
        printf("\n");
    }

    if (arena->alloc_list->size != 0) {
        dll_node_t *curr_block = arena->alloc_list->head;
        for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
            // curr_miniblock is the first miniblock in each block
            dll_node_t *curr_miniblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;

            uint64_t start = ((block_t *)curr_block->data)->start_address;
            uint64_t stop = ((block_t *)curr_block->data)->start_address + ((block_t *)curr_block->data)->size;
            printf("Block %d begin\n", i + 1);
            printf("Zone: 0x%lX - 0x%lX\n", start, stop);
            // Traverse all miniblocks
            for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {
                uint64_t miniblock_start = ((miniblock_t *)curr_miniblock->data)->start_address;
                uint64_t miniblock_stop = miniblock_start + ((miniblock_t *)curr_miniblock->data)->size;
                printf("Miniblock %d:\t\t0x%lX\t\t-\t\t0x%lX\t\t| ", j + 1, miniblock_start, miniblock_stop);
                if (((miniblock_t *)curr_miniblock->data)->perm == 0) {
                    printf("---\n");
                }
                if (((miniblock_t *)curr_miniblock->data)->perm == 1) {
                    printf("--X\n");
                }
                if (((miniblock_t *)curr_miniblock->data)->perm == 2) {
                    printf("-W-\n");
                }
                if (((miniblock_t *)curr_miniblock->data)->perm == 3) {
                    printf("-WX\n");
                }
                if (((miniblock_t *)curr_miniblock->data)->perm == 4) {
                    printf("R--\n");
                }
                if (((miniblock_t *)curr_miniblock->data)->perm == 5) {
                    printf("R-X\n");
                }
                if (((miniblock_t *)curr_miniblock->data)->perm == 6) {
                    printf("RW-\n");
                }
                if (((miniblock_t *)curr_miniblock->data)->perm == 7) {
                    printf("RWX\n");
                }

                curr_miniblock = curr_miniblock->next;
            }
            printf("Block %d end\n", i + 1);
            if (i + 1 != arena->alloc_list->size)
                printf("\n");
            curr_block = curr_block->next;
        }
    }
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
    int ok = 0; // set to 1 if the address is in any block
    int valid_adr = 0; // set if the given address is the start address of any miniblock
    dll_node_t *curr_block = arena->alloc_list->head;

    if (address >= arena->arena_size) {
        printf("Invalid address for mprotect.\n");
        return;
    }
    if (arena->alloc_list->size == 0 || address < ((block_t *)arena->alloc_list->head->data)->start_address) {
        printf("Invalid address for mprotect.\n");
        return;
    }
    else if (address > ((miniblock_t *)((list_t *)((block_t *)arena->alloc_list->head->prev->data)->miniblock_list)->head->prev->data)->start_address) {
        printf("Invalid address for mprotect.\n");
        return;
    }

    for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
        dll_node_t *curr_mblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;

        uint64_t start = ((block_t *)curr_block->data)->start_address; // where the block starts
        uint64_t stop = start + ((block_t *)curr_block->data)->size;   // where the block ends

        if (start <= address && address < stop) // choose the block containing the given address
        {
            ok = 1;
            // check if the address is the start of any miniblock
            for (unsigned int k = 0; k < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; k++) {
                uint64_t begin = ((miniblock_t *)curr_mblock->data)->start_address;
                if (address == begin) {
                    valid_adr = 1;
                }
                curr_mblock = curr_mblock->next;
            }
            if (valid_adr == 0) {
                printf("Invalid address for mprotect.\n");
                return;
            }

            curr_mblock = ((list_t *)((block_t *)curr_block->data)->miniblock_list)->head;
            for (unsigned int j = 0; j < ((list_t *)((block_t *)curr_block->data)->miniblock_list)->size; j++) {

                uint64_t mblock_start = ((miniblock_t *)curr_mblock->data)->start_address;
                if (address == mblock_start) {
                    ((miniblock_t *)curr_mblock->data)->perm = 0; // set all permissions to 0
                    char *permisiune = strtok((char *)permission, " |");

                    while (permisiune) {
                        if (strncmp(permisiune, "PROT_READ", 9) == 0) {
                            ((miniblock_t *)curr_mblock->data)->perm += 4;
                        }
                        else if (strncmp(permisiune, "PROT_WRITE", 10) == 0) {
                            ((miniblock_t *)curr_mblock->data)->perm += 2;
                        }
                        else if (strncmp(permisiune, "PROT_EXEC", 9) == 0) {
                            ((miniblock_t *)curr_mblock->data)->perm++;
                        }
                        else if (strncmp(permisiune, "PROT_NONE", 9) == 0) {
                            ((miniblock_t *)curr_mblock->data)->perm = 0;
                            return;
                        }
                        permisiune = strtok(NULL, " |");
                    }
                    return;
                }
                curr_mblock = curr_mblock->next;
            }
        }
        else {
            curr_block = curr_block->next; // if the input address is not between start and stop, move to the next block
        }
    }
    // means that the address is between blocks
    if (ok == 0) {
        printf("Invalid address for mprotect.\n");
        return;
    }
}
