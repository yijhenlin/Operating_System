
#include "hw_malloc.h"




Chunk_header *create_chunk_header(Chunk_header *input, int size)
{
    if((void *)input-start_brk+size > 0x00010000) {
        //printf("create chunk failed\n");
        return NULL;
    }
    Chunk_header *header = input;
    header->prev = header;
    header->next = header;
    header->size_and_flag.Allocated_flag = 0;
    header->size_and_flag.mmap_flag = 0;
    header->size_and_flag.CurrentChunk_size = size;
    header->size_and_flag.PrevChunk_size = 0;
    return header;
}

Chunk_header *remove_chunk(Chunk_header *input)
{
    ((Chunk_header *)input->next)->prev = input->prev;
    ((Chunk_header *)input->prev)->next = input->next;
    input->prev = input;
    input->next = input;
    return input;
}

void merge(Chunk_header *input1, Chunk_header *input2)
{

    if((void *)input1 < (void *)input2) {
        input1->size_and_flag.CurrentChunk_size *= 2;
        Chunk_header *base = remove_chunk(input1);
        remove_chunk(input2);
        input2 = NULL;
        int index = find_bin(input1->size_and_flag.CurrentChunk_size);
        inqueue(binptr[index], input1, 0);
    } else {
        input2->size_and_flag.CurrentChunk_size *= 2;
        Chunk_header *base = remove_chunk(input2);
        remove_chunk(input1);
        input1 = NULL;
        int index = find_bin(input2->size_and_flag.CurrentChunk_size);
        inqueue(binptr[index], input2, 0);
    }

}

Chunk_header *split(Chunk_header **input, int bytes)
{
    Chunk_header *base = *input;
    int size = base->size_and_flag.CurrentChunk_size;
    if(size < bytes) {
        //printf("not enough memory\n");
        return NULL;
    }
    size = size/2;
    //printf("size = %d\n",size);
    //printf("%p\t%d\n",(void *)base+size,base->size_and_flag.CurrentChunk_size-size);
    Chunk_header *temp = create_chunk_header((void *)base+size,base->size_and_flag.CurrentChunk_size-size);
    *input = temp;
    Chunk_header *new = create_chunk_header(base, size);
    return new;
}

void inqueue(Bin *bin, Chunk_header *chunk, int method) //method 0 add in bin 1 add in allocated/mmap
{
    Bin *head = bin;
    bin->size++;
    int size = chunk->size_and_flag.CurrentChunk_size;
    if(method == 0) {
        while(bin->next != head) {
            bin = bin->next;
            //printf("%p\t%p\t%d\n",bin,chunk,abs((void *)bin-(void *)chunk));
            if(abs((void *)bin-(void *)chunk) == size) {
                if(size != 32768) {
                    merge(chunk, (Chunk_header *)bin);
                    return;
                }
            }

        }
        chunk->next = head;
        chunk->prev = bin;
        chunk->size_and_flag.PrevChunk_size = size;
        head->prev = chunk;
        bin->next = chunk;
    } else {
        while(bin->next != head && ((Chunk_header *)bin->next)->size_and_flag.CurrentChunk_size <= size) {
            bin = bin->next;
        }
        chunk->next = bin->next;
        chunk->prev = bin;
        ((Chunk_header *)bin->next)->prev = chunk;
        bin->next = chunk;
    }
}

Chunk_header *dequeue(Bin *bin, void *free_chunk)   //free_chunk 0 choose min address
{
    if(bin->next == bin) {
        //printf("no chunk\n");
        return NULL;
    }
    Bin *head = bin;
    bin->size--;
    Chunk_header *remo = NULL;
    while(bin->next != head) {
        bin = bin->next;
        if(free_chunk != 0) {
            if((void *)bin == free_chunk) {
                remo = (Chunk_header *)bin;
                break;
            }
        } else {
            if(remo == NULL)remo = (Chunk_header *)bin;
            else if((void *)remo > (void *)bin)remo = (Chunk_header *)bin;
        }
    }
    if(remo == NULL)return remo;
    ((Chunk_header *)remo->prev)->next = remo->next;
    ((Chunk_header *)remo->next)->prev = remo->prev;
    remo->prev = remo;
    remo->next = remo;
    return remo;

}

void print_bin(int index)
{
    Bin *head = binptr[index];
    Bin *bin = binptr[index];
    bin = bin->next;
    while(bin != head) {
        void *mem = (void *)bin-start_brk;
        if(mem == 0)printf("0x000000000000--------%d\n",((Chunk_header *)bin)->size_and_flag.CurrentChunk_size);
        else printf("%014p--------%d\n",mem,((Chunk_header *)bin)->size_and_flag.CurrentChunk_size);
        bin = bin->next;
    }
}

void print_mmap(int flag)
{
    if(flag == 0) {
        Bin *head = allocate;
        allocate = allocate->next;
        while(allocate != head) {
            printf("%014p--------%d\n",(void *)allocate,((Chunk_header *)allocate)->size_and_flag.CurrentChunk_size);
            allocate = allocate->next;
        }
    } else if(flag == 1) {
        Bin *head = mmapptr;
        mmapptr = mmapptr->next;
        while(mmapptr != head) {
            printf("%014p--------%d\n",(void *)mmapptr,((Chunk_header *)mmapptr)->size_and_flag.CurrentChunk_size);
            mmapptr = mmapptr->next;
        }
    }
}

int find_bin(int size)
{
    switch(size) {
    case 32:
        return 0;
    case 64:
        return 1;
    case 128:
        return 2;
    case 256:
        return 3;
    case 512:
        return 4;
    case 1024:
        return 5;
    case 2048:
        return 6;
    case 4096:
        return 7;
    case 8192:
        return 8;
    case 16384:
        return 9;
    case 32768:
        return 10;
    default:
        return -1;
    }
}


void *hw_malloc(size_t bytes)
{

    Chunk_header *chunk1;
    Chunk_header *chunk2;
    Chunk_header *mmap_chunk;
    if(first == 0) {
        first = 1;
        allocate = &allocate_head;
        allocate->prev = allocate;
        allocate->next = allocate;
        mmapptr = &mmap_head;
        mmapptr->prev = mmapptr;
        mmapptr->next = mmapptr;
        start_brk = sbrk(0);
        int f = brk(start_brk+0x00010000);
        if(f < 0);//printf("allocate error\n");
        for(int i = 0; i < 11; i++) {
            binptr[i] = &bin_head[i];
            binptr[i]->size = 0;
            binptr[i]->prev = binptr[i];
            binptr[i]->next = binptr[i];
        }
        chunk1 = create_chunk_header(start_brk, 0x00010000);
        if(bytes <= mmap_threshold) {
            while(chunk1->size_and_flag.CurrentChunk_size/2 >= bytes+24) {
                chunk2 = split(&chunk1,bytes);
                int num = find_bin(chunk1->size_and_flag.CurrentChunk_size);
                inqueue(binptr[num], chunk1, 0);
                chunk1 = chunk2;
            }
            chunk2->size_and_flag.Allocated_flag = 1;
            inqueue(allocate, chunk2, 1);
            return (void *)chunk2+24;
        } else {
            chunk2 = split(&chunk1,bytes);
            int num = find_bin(chunk1->size_and_flag.CurrentChunk_size);
            inqueue(binptr[num], chunk1, 0);
            inqueue(binptr[num], chunk2, 0);
            mmap_chunk = mmap(NULL,bytes+24,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
            if(mmap_chunk < 0)return NULL;
            else {
                mmap_chunk->size_and_flag.CurrentChunk_size = bytes+24;
                mmap_chunk->size_and_flag.mmap_flag = 1;
                inqueue(mmapptr,mmap_chunk, 1);
            }
            return (void *)mmap_chunk+24;
        }
    } else {
        if(bytes + 24 <= mmap_threshold) {
            int n;
            for(n = 32768; n >= 32; n = n/2) {
                if(n < bytes+24)break;
            }
            int index = find_bin(n*2);
            for(; index < 11; index++) {
                chunk1 = dequeue(binptr[index],0);
                if(chunk1 != NULL) {
                    while(chunk1->size_and_flag.CurrentChunk_size/2 >= bytes+24) {
                        chunk2 = split(&chunk1,bytes);
                        int num = find_bin(chunk1->size_and_flag.CurrentChunk_size);
                        inqueue(binptr[num], chunk1, 0);
                        chunk1 = chunk2;
                    }
                    chunk1->size_and_flag.Allocated_flag = 1;
                    inqueue(allocate, chunk1, 1);
                    return (void *)chunk1+24;
                }
            }
            //printf("malloc error\n");
            return NULL;
        } else {
            Chunk_header *mmap_chunk = mmap(NULL,bytes+24,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
            if(mmap_chunk < 0)return NULL;
            else {
                mmap_chunk->size_and_flag.CurrentChunk_size = bytes+24;
                mmap_chunk->size_and_flag.mmap_flag = 1;
                inqueue(mmapptr, mmap_chunk, 1);
            }
            return (void *)mmap_chunk+24;
        }
    }

}

int hw_free(void *mem)
{
    //printf("%p\n",mem);
    Chunk_header *free_chunk = mem;
    if(free_chunk->size_and_flag.Allocated_flag == 1) {
        free_chunk = dequeue(allocate, mem);
        if(free_chunk == NULL)return 0;
        free_chunk->size_and_flag.Allocated_flag = 0;
        int index = find_bin(free_chunk->size_and_flag.CurrentChunk_size);
        inqueue(binptr[index], free_chunk, 0);
        return 1;
    }
    if(free_chunk->size_and_flag.mmap_flag == 1) {
        free_chunk = dequeue(mmapptr, mem);
        if(free_chunk == NULL)return 0;
        free_chunk->size_and_flag.mmap_flag = 0;
        munmap(free_chunk,free_chunk->size_and_flag.CurrentChunk_size);
        return 1;
    } else return 0;
}

void *get_start_sbrk(void)
{
    return start_brk;
}

