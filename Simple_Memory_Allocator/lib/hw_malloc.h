#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#define mmap_threshold 32*1024
#define header_size 24
void *start_brk;
int first;

typedef void* chunk_ptr_t;
typedef struct {
    unsigned PrevChunk_size : 31;
    unsigned Allocated_flag : 1;
    unsigned CurrentChunk_size : 31;
    unsigned mmap_flag : 1;
} chunk_info_t;

typedef struct {
    chunk_ptr_t prev;
    chunk_ptr_t next;
    chunk_info_t size_and_flag;
} Chunk_header;


typedef struct {
    chunk_ptr_t prev;
    chunk_ptr_t next;
    int size;
} Bin;
Bin bin_head[11];
Bin *binptr[11];
Bin allocate_head;
Bin *allocate;
Bin mmap_head;
Bin *mmapptr;
Chunk_header *create_chunk_header(Chunk_header *input, int size);
Chunk_header *remove_chunk(Chunk_header *input);
void merge(Chunk_header *input1, Chunk_header *input2);
Chunk_header *split(Chunk_header **input, int bytes);
void inqueue(Bin *bin, Chunk_header *chunk, int method);
Chunk_header *dequeue(Bin *bin, void *free_chunk);
void print_bin(int index);
void print_mmap(int flag);
int find_bin(int size);
void *hw_malloc(size_t bytes);
int hw_free(void *mem);
void *get_start_sbrk(void);

#endif
