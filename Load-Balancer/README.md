# Load Balancer

## Description:
A brief description of the functionality of the project: using singly linked lists, the program allocates memory for an array, `hash_ring`, where I will subsequently place servers that contain keys and values.

## Additional Explanations:
Below is a brief overview of the functionalities of the functions in `server.c` and `load_balancer.c` that may not be clear from the code alone.

---

## I. SERVER.C

### Function Descriptions:
- **`init_server_memory`**: Creates memory for a server. The variable `server` is a hashtable of type `server_memory`.

- **`server_store`**: Stores the data received from the Load Balancer in a hashtable.

- **`server_retrieve`**: Returns the value associated with the key from the hashtable after finding the appropriate node.

- **`server_remove`**: Deletes the node containing the key and its associated value from the correct server.

- **`free_server_memory`**: Frees a server by traversing all the buckets in the hashtable.

---

## II. LOAD_BALANCER.C

### Function Descriptions:
- **`init_load_balancer`**: Allocates the load balancer `main` and the array `hash_ring`.

- **`loader_add_server`**: 
  - First, I handle the case where the array is empty and add the first server along with its copies.
  - I add them one by one and interchange them based on their hash values so that they are in ascending order.
  - I then check if there are other servers in the array. After allocating space for the three servers, I use a for loop to add each of the three servers in turn. 
  - The variable `num`, initially equal to 3, is used to determine the position where the new server should be placed.
  - I first check if the added server is the first one, and if so, I copy the keys from the next server that have a hash less than the new server's hash.
  - I then handle the case where the added server is the last one, followed by the case where the added server is somewhere between existing servers.

- **`loader_remove_server`**: 
  - Checks both the original server and its copies. 
  - All elements on the server I want to delete are moved to the next server.
  - I traverse the buckets of the server being deleted and the entire list within each bucket.
  - I first check if I am deleting the last server, then if I am deleting a server that is not the last.

- **`loader_store`**: 
  - Finds the server where the key should be stored.
  - I first handle the case where the key is before the first server, then the case where the key is after the last server, and finally the case where the key is between servers.

- **`loader_retrieve`**: 
  - Calculates which server the key is stored on and retrieves its value, using the same cases as in `loader_store`.

- **`free_load_balancer`**: 
  - Traverses my array and frees all allocated memory.

---

# More info about the functionality of the project:
https://ocw.cs.pub.ro/courses/sd-ca/teme/tema2-2021
