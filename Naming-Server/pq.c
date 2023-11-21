#include "includes.h"

heap_t* create_heap (int capacity) {
  heap_t* heap = malloc(sizeof(heap_t));
  assert(heap != NULL);

  heap->capacity = capacity + 1;
  heap->num_el = 0;

  heap->arr = malloc((capacity + 1) * sizeof(int));
  for (int i = 0; i <= capacity; i++) heap->arr[i] = INT_MAX;
  assert(heap->arr != NULL);

  return heap;
}

heap_t* fix_heap (heap_t* heap, int node_pos) {
  int left_pos = node_pos * 2;
  int right_pos = left_pos + 1;

  if (node_pos > heap->num_el) return heap;
#ifndef ONLINE_JUDGE
  printf("node_pos = %d, left_pos = %d, right_pos = %d\n", node_pos, left_pos, right_pos);
#endif

  int smallest_pos = node_pos;
  if (left_pos <= heap->num_el && heap->arr[node_pos] > heap->arr[left_pos]) {
    smallest_pos = left_pos;
  }
  if (right_pos <= heap->num_el && heap->arr[smallest_pos] > heap->arr[right_pos]) {
    smallest_pos = right_pos;
  }

#ifndef ONLINE_JUDGE
  printf("heap->arr[smallest_pos] = %d\n", heap->arr[smallest_pos]);
#endif

  if (smallest_pos != node_pos) {
    int temp = heap->arr[node_pos];
    heap->arr[node_pos] = heap->arr[smallest_pos];
    heap->arr[smallest_pos] = temp;
    fix_heap(heap, smallest_pos);
  }

  return heap;
}

int is_heap_full (heap_t* heap) {
  return heap->num_el >= heap->capacity;
}

heap_t* insert_into_heap (heap_t* heap, int element) {
  heap->num_el++;
  assert(!is_heap_full(heap));
  heap->arr[heap->num_el] = element;
  int pos = heap->num_el;
  while (pos > 1 && heap->arr[pos / 2] > heap->arr[pos]) {   
    int temp = heap->arr[pos];
    heap->arr[pos] = heap->arr[pos / 2];
    heap->arr[pos / 2] = temp;
    pos /= 2;
  }
  return heap;
}

heap_t* heapify (int* arr, int len) {
  heap_t* heap = create_heap(len);
  for (int i = 1; i <= len; i++) heap->arr[i] = arr[i - 1];
  heap->num_el = len;
  
  for (int i = (heap->num_el - 1) / 2; i > 0; i--) {
    fix_heap(heap, i);
  }
  
  return heap;
}

int is_heap_empty (heap_t* heap) {
  return heap->num_el <= 0;
}

int extract_min (heap_t *heap) {
  assert(!is_heap_empty(heap));
  int min = heap->arr[1];

  heap->arr[1] = heap->arr[heap->num_el];
  heap->arr[heap->num_el] = INT_MAX;
  heap->num_el--;
  fix_heap(heap, 1);

#ifndef ONLINE_JUDGE
  print_heap(heap);
#endif
  return min;
}

void print_heap (heap_t* heap) {
  if (heap->num_el > 0) {
    printf("[");
    for (int i = 1; i < heap->num_el; i++) printf("%d, ", heap->arr[i]);
    printf("%d]\n", heap->arr[heap->num_el]);
  }
}

void free_heap (heap_t* heap) {
  free(heap->arr);
  free(heap);
}