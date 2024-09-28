#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

typedef struct server_details server_details;
struct server_details {
	int server_ID; // server ID
	unsigned int server_hash; // hash of the server
	server_memory *server; // pointer to a structure of type server_memory
};

typedef struct load_balancer load_balancer;
struct load_balancer {
    unsigned int size; // size of the hash_ring array
	server_details **hash_ring; // pointer to my array
};

// receives the server ID and returns the corresponding hash
unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

// receives the key "Sapphire" and hashes it
// this hash is also a number
unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

load_balancer *init_load_balancer() {
	load_balancer *main;
	main = malloc(sizeof(load_balancer));
	main->size = 0;
	main->hash_ring = malloc(sizeof(server_details*));
	if (main->hash_ring == NULL)
	{
		printf("Could not allocate memory");
		exit(0);
	}
    return main;
}

void loader_add_server(load_balancer *main, int server_id) {
    // increase the size of the hash_ring by 3
    main->size = main->size + 3;
    main->hash_ring = realloc(main->hash_ring, main->size * sizeof(server_details *));

    // allocate space for the 3 servers, and then
    // check where to place them on the ring
    for (int i = main->size - 3; i < main->size; i++)
    {
        main->hash_ring[i] = malloc(sizeof(server_details));
        main->hash_ring[i]->server = init_server_memory();
    }

    // check if the hash_ring is empty
    if (main->size == 3)
    {
        // create the original server
        unsigned long server_hash0 = hash_function_servers(&server_id);
        main->hash_ring[0]->server_hash = server_hash0;
        main->hash_ring[0]->server_ID = server_id;
        // make copies of server_hash0 and server_id
        unsigned long copie_hash0 = server_hash0;
        unsigned long copie_id0 = server_id;

        // first replica of the server
        server_id += 100000;
        unsigned long server_hash1 = hash_function_servers(&server_id);
        if (server_hash1 > server_hash0)
        {
            main->hash_ring[1]->server_hash = server_hash1;
            main->hash_ring[1]->server_ID = server_id;
        }
        else if (server_hash1 < server_hash0)
        {
            // place the first server in position 1
            main->hash_ring[1]->server_hash = copie_hash0;
            main->hash_ring[1]->server_ID = copie_id0;
            // place the first replica in position 0
            main->hash_ring[0]->server_hash = server_hash1;
            main->hash_ring[0]->server_ID = server_id;
        }
        // make copies of server_hash1 and server_id
        unsigned long copie_hash1 = server_hash1;
        unsigned long copie_id1 = server_id;

        // create the second replica of the server
        server_id += 100000;
        unsigned long server_hash2 = hash_function_servers(&server_id);
        if (server_hash2 > server_hash0 && server_hash2 > server_hash1)
        {
            main->hash_ring[2]->server_hash = server_hash2;
            main->hash_ring[2]->server_ID = server_id;
        }
        else if (server_hash2 < server_hash0 && server_hash2 < server_hash1)
        {
            // move the second to position 3
            main->hash_ring[2]->server_hash = copie_hash1;
            main->hash_ring[2]->server_ID = copie_id1;

            // move the first to position 2
            main->hash_ring[1]->server_hash = copie_hash0;
            main->hash_ring[1]->server_ID = copie_id0;

            // place the new one in position 0
            main->hash_ring[0]->server_hash = server_hash2;
            main->hash_ring[0]->server_ID = server_id;
        }
        else if (server_hash2 > server_hash0 && server_hash2 < server_hash1)
        {
            main->hash_ring[2]->server_hash = copie_hash1;
            main->hash_ring[2]->server_ID = copie_id1;

            main->hash_ring[1]->server_hash = server_hash2;
            main->hash_ring[1]->server_ID = server_id;
        }
    }
    else if (main->size != 0)
    {
        int num = 3;
        int ok1 = 0;
        int ok2 = 0;
        unsigned long server_hash = hash_function_servers(&server_id);

        for (int k = 0; k < 3; k++)
        {
            // case where the added server is before all others
            if (server_hash < main->hash_ring[0]->server_hash) {
                for (unsigned int j = main->size - 1; j > 0; j--)
                    main->hash_ring[j] = main->hash_ring[j - 1];
                main->hash_ring[0]->server_hash = server_hash;
                main->hash_ring[0]->server_ID = server_id;
                // go to the next server and see which keys have a smaller hash than my server's hash
                // these will be placed in the new server
                for (unsigned int i = 0; i < main->hash_ring[1]->server->hmax; i++)
                {
                    ll_node_t *curr = main->hash_ring[1]->server->buckets[i]->head;
                    // traverse the bucket list
                    for (unsigned int h = 0; h < main->hash_ring[1]->server->buckets[i]->size; h++)
                    {
                        if (server_hash > hash_function_key(((info *)curr->data)->key))
                        {
                            server_store(main->hash_ring[0]->server, ((info *)curr->data)->key, ((info *)curr->data)->value);
                            server_remove(main->hash_ring[1]->server, ((info *)curr->data)->key);
                        }
                        curr = curr->next;
                    }
                }
            }
            // means it is after the last server in the array
            else if (server_hash > main->hash_ring[main->size - num]->server_hash) {
                // place the new server at the last position
                main->hash_ring[main->size - num]->server_hash = server_hash;
                main->hash_ring[main->size - num]->server_ID = server_id;
                // go to the first server and check
                for (unsigned int i = 0; i < main->hash_ring[0]->server->hmax; i++) {
                    ll_node_t *curr = main->hash_ring[0]->server->buckets[i]->head;
                    // traverse the bucket list
                    for (unsigned int h = 0; h < main->hash_ring[0]->server->buckets[i]->size; h++) {
                        if (server_hash > hash_function_key(((info *)curr->data)->key)) {
                            server_store(main->hash_ring[main->size - num]->server, ((info *)curr->data)->key, ((info *)curr->data)->value);
                            server_remove(main->hash_ring[0]->server, ((info *)curr->data)->key);
                        }
                        curr = curr->next;
                    }
                }
            }
            else
            {
                for (unsigned int i = 0; i < main->size; i++)
                {
                    // found the position to insert the original server 
                    if (server_hash > main->hash_ring[i]->server_hash && server_hash < main->hash_ring[i + 1]->server_hash)
                    {
                        for (unsigned int j = main->size - 1; j > i + 1; j--)
                            main->hash_ring[j] = main->hash_ring[j - 1];
                        main->hash_ring[i + 1]->server_hash = server_hash;
                        main->hash_ring[i + 1]->server_ID = server_id;
                        // go to the next server
                        for (unsigned int i = 0; i < main->hash_ring[i + 2]->server->hmax; i++) {
                            ll_node_t *curr = main->hash_ring[i + 2]->server->buckets[i]->head;
                            // traverse the bucket list
                            for (unsigned int h = 0; h < main->hash_ring[i + 2]->server->buckets[i]->size; h++) {
                                if (server_hash > hash_function_key(((info *)curr->data)->key)) {
                                    server_store(main->hash_ring[i + 1]->server, ((info *)curr->data)->key, ((info *)curr->data)->value);
                                    server_remove(main->hash_ring[i + 2]->server, ((info *)curr->data)->key);
                                }
                                curr = curr->next;
                            }
                        }
                    }
                }
            }

            num--;
            // means I am making the first copy
            if (ok1 == 0 && ok2 == 0)
            {
                ok1 = 1;
                server_hash = hash_function_servers((&server_id) + 100000);
            }
            // means I am making the second copy
            else if (ok1 == 1 && ok2 == 0)
            {
                ok2 = 1;
                server_hash = hash_function_servers((&server_id) + 100000);
            }
        }
    }
}

void loader_remove_server(load_balancer *main, int server_id) {
    for (unsigned int i = 0; i < main->size; i++) {
        // Check both the original server and its copies
        if (server_id == main->hash_ring[i]->server_ID % 100000) {
            // Transfer all elements from the server being removed to the next server in the ring
            for (unsigned int j = 0; j < main->hash_ring[i]->server->hmax; j++) {
                ll_node_t *curr = main->hash_ring[i]->server->buckets[j]->head;
                // Traverse the entire list in the bucket
                while (curr) {
                    // If removing the last server, store to the first server
                    if (i == main->size - 1) {
                        server_store(main->hash_ring[0]->server, ((info *)curr->data)->key, ((info *)curr->data)->value);
                    } else {
                        // Store to the next server in the ring
                        server_store(main->hash_ring[i + 1]->server, ((info *)curr->data)->key, ((info *)curr->data)->value);
                    }
                    curr = curr->next; // Move to the next node
                }
            }

            // Free memory for the server being removed
            free_server_memory(main->hash_ring[i]->server);
            free(main->hash_ring[i]);

            // Shift remaining servers down in the hash ring
            for (unsigned int j = i; j < main->size - 1; j++) {
                main->hash_ring[j] = main->hash_ring[j + 1];
            }

            // Decrease the size of the hash ring
            main->size--;
            i--; // Decrement 'i' to check the new server at the current index
        }
    }

    // Resize the hash_ring array to fit the new size
    if (main->size > 0) {
        main->hash_ring = realloc(main->hash_ring, main->size * sizeof(server_details *));
    } else {
        // If no servers are left, free the hash_ring pointer
        free(main->hash_ring);
        main->hash_ring = NULL;
    }
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id) {
    // Store the server ID where the key is stored
    // Calculate the hash of the key
    unsigned int hash = hash_function_key(key);

    // Case where the key's hash is less than the first server's hash
    if (main->size > 0 && hash < main->hash_ring[0]->server_hash) {
        server_store(main->hash_ring[0]->server, key, value);
        *server_id = main->hash_ring[0]->server_ID;
        return; // Exit function after storing
    }

    // Case where the key's hash is greater than the last server's hash
    if (main->size > 0 && hash > main->hash_ring[main->size - 1]->server_hash) {
        server_store(main->hash_ring[0]->server, key, value);
        *server_id = main->hash_ring[0]->server_ID;
        return; // Exit function after storing
    }

    // Case where the key's hash is between the first and last server
    for (unsigned int i = 0; i < main->size - 1; i++) {
        if (hash > main->hash_ring[i]->server_hash && hash < main->hash_ring[i + 1]->server_hash) {
            server_store(main->hash_ring[i + 1]->server, key, value);
            *server_id = main->hash_ring[i + 1]->server_ID;
            return; // Exit function after storing
        }
    }

    // If the hash is equal to a server's hash, store it in that server (optional)
    for (unsigned int i = 0; i < main->size; i++) {
        if (hash == main->hash_ring[i]->server_hash) {
            server_store(main->hash_ring[i]->server, key, value);
            *server_id = main->hash_ring[i]->server_ID;
            return; // Exit function after storing
        }
    }
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
    // Calculate the hash of the key
    unsigned int hash = hash_function_key(key);

    // Case where the key's hash is less than the first server's hash
    if (main->size > 0 && hash < main->hash_ring[0]->server_hash) {
        *server_id = main->hash_ring[0]->server_ID;
        return server_retrieve(main->hash_ring[0]->server, key);
    }

    // Case where the key's hash is greater than the last server's hash
    if (main->size > 0 && hash > main->hash_ring[main->size - 1]->server_hash) {
        *server_id = main->hash_ring[main->size - 1]->server_ID;
        return server_retrieve(main->hash_ring[0]->server, key);
    }

    // Case where the key's hash is between the first and last server
    for (unsigned int i = 0; i < main->size - 1; i++) {
        if (hash > main->hash_ring[i]->server_hash && hash < main->hash_ring[i + 1]->server_hash) {
            *server_id = main->hash_ring[i + 1]->server_ID;
            return server_retrieve(main->hash_ring[i + 1]->server, key);
        }
    }

    // If the key was not found, return NULL
    return NULL;
}

void free_load_balancer(load_balancer *main) {
    // Iterate through the hash ring and free each server
    for (unsigned int i = 0; i < main->size; i++) {
        free_server_memory(main->hash_ring[i]->server); // Free server memory
        free(main->hash_ring[i]); // Free server details
    }
    free(main->hash_ring); // Free the hash ring
    free(main); // Free the load balancer itself
}

