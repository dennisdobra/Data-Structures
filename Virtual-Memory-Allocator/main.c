#include "vma.h"

int main(void)
{
	arena_t *arena;

	while (1)
	{
		char command[16];
		scanf("%s", command);
		long arena_size, address, size;

		if (strcmp(command, "ALLOC_ARENA") == 0) {
			scanf("%ld", &arena_size);
			arena = alloc_arena(arena_size);
		}
		else if (strcmp(command, "DEALLOC_ARENA") == 0) {
			dealloc_arena(arena);
			break;
		}
		else if (strcmp(command, "ALLOC_BLOCK") == 0) {
			scanf("%ld %ld", &address, &size);
			alloc_block(arena, address, size);
		}
		else if (strcmp(command, "FREE_BLOCK") == 0) {
			scanf("%ld", &address);
			free_block(arena, address);
		}
		else if (strcmp(command, "READ") == 0) {
			scanf("%ld %ld", &address, &size);
			read(arena, address, size);
		}
		else if (strcmp(command, "WRITE") == 0) {
			scanf("%ld %ld", &address, &size);
			int8_t *data = malloc(size + 1);
			for (unsigned int i = 0; i < size; i++) {
				data[i] = getc(stdin);
				if (i == 0 && data[i] == ' ') {
					data[i] = getc(stdin);
				}
			}
			write(arena, address, size, data);
			free(data);
		}
		else if (strcmp(command, "PMAP") == 0) {
			pmap(arena);
		}
		else if (strcmp(command, "MPROTECT") == 0) {
			scanf("%ld", &address);
			int8_t permission[200];
			fgets((char *)permission, 200, stdin);
			
			mprotect(arena, address, permission);

		}
		else {
			printf("Invalid command. Please try again.\n");
		}
	}
	return 0;
}
