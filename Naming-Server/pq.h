#ifndef __heap
#define __heap

typedef struct heap {
  int num_el;
  int capacity;
  int* arr;
} heap_t;

// methods required for min heap

heap_t* create_heap (int capacity);

heap_t* insert_into_heap (heap_t* heap, int element);

heap_t* heapify (int* arr, int len);

int extract_min (heap_t* heap);

int is_heap_empty (heap_t* heap);

void print_heap (heap_t* heap);

void free_heap (heap_t* heap);
#endif