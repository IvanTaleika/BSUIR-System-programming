#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <unistd.h>
#include <string.h>

class Allocator {
 public:
  Allocator();
  ~Allocator();
  void free(void* ptr);
  void* calloc(size_t size);
  void* malloc(size_t size);
  void* realloc(void* ptr, size_t newSize);
 private:
  struct MemoryChunkData {
    bool isAvalible;
    size_t size;
  };
  void* memoryAllocationStart;
  void* memoryAllocationEnd;
  const size_t additionalMemory = sizeof(MemoryChunkData) + sizeof(size_t);

  void mergeWithNext(MemoryChunkData* currentChunk);
  void mergeWithPrevious(MemoryChunkData* currentChunk);
  MemoryChunkData* splitChunk(MemoryChunkData* currentChunk, size_t firstChunkSize);
};


#endif // ALLOCATOR_H
