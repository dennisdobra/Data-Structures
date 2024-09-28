# Virtual Memory Allocator

## Project Overview
In this project, I implemented a Virtual Memory Allocator in C that simulates the behavior of memory allocation functions like `malloc()`, `calloc()`, and `free()`. This project deepened my understanding of C programming, data structures, and the concept of virtual memory.

## Objectives
- Enhance my knowledge of the C programming language.
- Implement and utilize a doubly linked list data structure.
- Familiarize myself with the implementation of a generic data structure.

The primary goal was to create a virtual memory allocator that operates solely using doubly linked lists, simulating the functionality of an operating system's memory allocation system.

## Requirements
The memory allocator supports the following functionalities (API):

1. **ALLOC_ARENA `<size>`**: Allocates a contiguous buffer of the specified size to be used as the kernel buffer (arena).
  
2. **DEALLOC_ARENA**: Frees the allocated arena and all resources associated with it.
  
3. **ALLOC_BLOCK `<start_address>` `<block_size>`**: Marks a memory area as allocated in the kernel buffer, checking for overlaps with previously allocated blocks.
  
4. **FREE_BLOCK `<start_address>`**: Frees a miniblock. If it is the only miniblock, the parent block is also freed.
  
5. **READ `<start_address>` `<size>`**: Reads specified bytes from the given address, issuing a warning if the size exceeds the block size.
  
6. **WRITE `<start_address>` `<size>` `<data>`**: Writes specified bytes at the given address, issuing a warning if the size exceeds the block size.
  
7. **PMAP**: Displays information about allocated blocks and miniblocks, including their sizes and permissions.

## Error Handling
I implemented the following error messages:

- **INVALID_ALLOC_BLOCK**: Triggered if the allocated address exceeds the arena size or overlaps with previously allocated areas.
- **INVALID_ADDRESS_FREE**: Triggered if an invalid address is provided for freeing memory.
- **INVALID_ADDRESS_READ**: Triggered if an invalid address is used for reading.
- **INVALID_ADDRESS_WRITE**: Triggered if an invalid address is used for writing.
- **INVALID_COMMAND**: Triggered for unrecognized commands or incorrect parameter counts.

## Bonus Features
As an additional feature, I implemented:

- **MPROTECT `<start_address>` `<new_permission>`**: Changes the permissions of a memory area based on specified flags.

## Conclusion
This project provides a practical application of data structures in C and simulates essential memory management concepts. By completing this project, I gained valuable experience in implementing and managing dynamic memory allocation systems.

## More info about the functionality of the project:
https://ocw.cs.pub.ro/courses/sd-ca/teme/tema1-2023
