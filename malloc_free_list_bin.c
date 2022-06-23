//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

typedef struct my_metadata_t {
  size_t size;
  struct my_metadata_t *next;
} my_metadata_t;

typedef struct my_heap_t {
  my_metadata_t *free_head;
  my_metadata_t dummy;
} my_heap_t;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
// my_heap_t my_heap;
my_heap_t my_bins[10];

//
// Helper functions (feel free to add/remove/edit!)
//

// binを探す
int find_bin(size_t size) {
  int bin_index, i;
  size_t min_size = 8;
  // ~8, ~16, ~32, ~64, ..., ~4096の10個の瓶
  for (i = 0; i < 10; i++) {
    if (size <= min_size) {
      bin_index = i;
      break;
    }
    min_size *= 2;
  }
  return bin_index;
}


void my_add_to_free_list(my_metadata_t *metadata) {
  assert(!metadata->next);
  int bin_index = 0;

  bin_index = find_bin(metadata->size);

  /*
  // 最初のif文ゴリ押し
  if (metadata->size <= 1024) bin_index = 0;
  else if (1024 < metadata->size && metadata->size <=2048) bin_index = 1;
  else if (2048 < metadata->size && metadata->size <=3072) bin_index = 2;
  else bin_index = 3;
  */

  metadata->next = my_bins[bin_index].free_head;
  my_bins[bin_index].free_head = metadata;
}

void my_remove_from_free_list(my_metadata_t *metadata, my_metadata_t *prev, int bin_index) {
  if (prev) {
    prev->next = metadata->next;
  } else {
    my_bins[bin_index].free_head = metadata->next;
  }
  metadata->next = NULL;
}

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
void my_initialize() {
  int i;
  for(i = 0; i < 10; i++) {
    my_bins[i].free_head = &my_bins[i].dummy;
    my_bins[i].dummy.size = 0;
    my_bins[i].dummy.next = NULL;
  }
  /*
  my_heap.free_head = &my_heap.dummy;
  my_heap.dummy.size = 0;
  my_heap.dummy.next = NULL;
  */
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size) {

  // binを探す
  int i, bin_index = 0;
  bin_index = find_bin(size);

  my_metadata_t *metadata = NULL;
  my_metadata_t *prev = NULL;
  my_metadata_t *min_metadata = NULL;
  my_metadata_t *min_prev = NULL;
  size_t min_size;

  // best-fitで空き領域を探す
  for (i = bin_index; i < 10; i++) {
    min_size = 4906;
    metadata = my_bins[i].free_head;
    prev = NULL;
    min_metadata = NULL;
    min_prev = NULL;

    while (metadata) {
      if (metadata->size >= size) {
        if (metadata->size < min_size) {
          min_size = metadata->size;
          min_metadata = metadata;
          min_prev = prev;
        }
      }
      prev = metadata;
      metadata = metadata->next;
    }
    if (min_metadata) {
      // printf("%zu %zu %d\n", min_metadata->size, min_size, i);
      break;
    }
  }
  
  // 地味に値を更新
  bin_index = i;
  prev = min_prev;
  metadata = min_metadata;

  // now, metadata points to the first free slot
  // and prev is the previous entry.

  if (!metadata) {
    // There was no free slot available. We need to request a new memory region
    // from the system by calling mmap_from_system().
    //
    //     | metadata | free slot |
    //     ^
    //     metadata
    //     <---------------------->
    //            buffer_size
    size_t buffer_size = 4096;
    my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
    metadata->size = buffer_size - sizeof(my_metadata_t); //?????
    metadata->next = NULL;
    // Add the memory region to the free list.
    my_add_to_free_list(metadata);
    // Now, try my_malloc() again. This should succeed.
    return my_malloc(size);
  }

  // |ptr| is the beginning of the allocated object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  void *ptr = metadata + 1;
  size_t remaining_size = metadata->size - size;
  metadata->size = size;
  // Remove the free slot from the free list.
  my_remove_from_free_list(metadata, prev, bin_index);

  if (remaining_size > sizeof(my_metadata_t)) {
    // Create a new metadata for the remaining free slot.
    //
    // ... | metadata | object | metadata | free slot | ...
    //     ^          ^        ^
    //     metadata   ptr      new_metadata
    //                 <------><---------------------->
    //                   size       remaining size
    my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
    new_metadata->size = remaining_size - sizeof(my_metadata_t);
    new_metadata->next = NULL;
    // Add the remaining free slot to the free list.
    my_add_to_free_list(new_metadata);
  }
  return ptr;
}

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr) {
  // Look up the metadata. The metadata is placed just prior to the object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
  // Add the free slot to the free list.
  my_add_to_free_list(metadata);
}

// This is called at the end of each challenge.
void my_finalize() {
  // Nothing is here for now.
  // feel free to add something if you want!
}

void test() {
  // Implement here!
  assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
}
