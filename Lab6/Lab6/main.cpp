#include "allocator.h"
#include "stdio.h"
#define ARRAY_SIZE 5
#define CELL_SIZE 10
#define BIG_CELL_SIZE 30
#define SMALL_CELL_SIZE 5


int main(void) {
  Allocator myAllocator;
  int** ptr = (int**) myAllocator.malloc(ARRAY_SIZE * sizeof(int*));
  int counter = 0;
  for (int i = 0; i < ARRAY_SIZE; i++) {
    ptr[i] = (int*) myAllocator.malloc(CELL_SIZE * sizeof(int));
    for (int j = 0; j < CELL_SIZE; j++) {
      ptr[i][j] = counter;
      counter++;
    }
  }
  myAllocator.free(ptr[2]);
  myAllocator.free(ptr[3]);
  myAllocator.free(ptr[1]);
  ptr[2] = (int*) myAllocator.malloc(BIG_CELL_SIZE * sizeof(int));
  for (int i = 0; i < BIG_CELL_SIZE; i++) {
    ptr[2][i] = counter;
    counter++;
  }
  ptr[3] = (int*) myAllocator.malloc(SMALL_CELL_SIZE * sizeof(int));
  for (int i = 0; i < SMALL_CELL_SIZE; i++) {
    ptr[3][i] = counter;
    counter++;
  }
  ptr[2] = (int*)myAllocator.realloc(ptr[2], 1 * sizeof(int));
  ptr[2] = (int*)myAllocator.realloc(ptr[2], BIG_CELL_SIZE * sizeof(int));
  int* ptr1 = (int*)myAllocator.calloc(10 * sizeof(int));
  for (int i = 0; i < 10; i++) {
    printf("%d\n", ptr1[i]);
  }
  return 0;
}
